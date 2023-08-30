#include <pbr/exposure.inc>
#include <pbr/exposure_pass.hpp>
#include <av_assert.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr float DEFAULT_EYE_ADAPTATION_SPEED = 1.0F;

constexpr float DEFAULT_EXPOSURE_COMPENSATION_EV = -10.0F;
constexpr float DEFAULT_MIN_LUMA_EV = 0.52F;
constexpr float DEFAULT_MAX_LUMA_EV = 15.0F;

} // end of anonymous

//----------------------------------------------------------------------------------------------------------------------

void ExposurePass::Execute ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept
{
    if ( _isNeedTransitLayout )
        TransitSync5Mip ( commandBuffer );

    _program.Bind ( commandBuffer );

    _exposureInfo._eyeAdaptation = EyeAdaptationFactor ( deltaTime );
    _program.SetPushConstants ( commandBuffer, &_exposureInfo );

    _program.SetDescriptorSet ( commandBuffer, _descriptorSet );

    vkCmdDispatch ( commandBuffer, _dispatch.width, _dispatch.height, _dispatch.depth );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        1U,
        &_exposureBarrier,
        0U,
        nullptr
    );
}

void ExposurePass::FreeTransferResources ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    vkFreeCommandBuffers ( device, commandPool, 1U, &_commandBuffer );
    _commandBuffer = VK_NULL_HANDLE;
    FreeTransferBuffer ( renderer, device );
}

VkBuffer ExposurePass::GetExposure () const noexcept
{
    return _exposureBarrier.buffer;
}

bool ExposurePass::Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    _eyeAdaptationSpeed = DEFAULT_EYE_ADAPTATION_SPEED;

    _exposureInfo._exposureCompensation = ExposureValueToLuma ( DEFAULT_EXPOSURE_COMPENSATION_EV );
    _exposureInfo._minLuma = ExposureValueToLuma ( DEFAULT_MIN_LUMA_EV );
    _exposureInfo._maxLuma = ExposureValueToLuma ( DEFAULT_MAX_LUMA_EV );

    return StartCommandBuffer ( commandPool, device ) &&
        CreateGlobalCounter ( renderer, device ) &&
        CreateDescriptorSet ( device ) &&
        CreateExposureResources ( renderer, device ) &&
        CreateLumaResources ( renderer, device ) &&
        SubmitCommandBuffer ( renderer );
}

void ExposurePass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _globalCounter != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _globalCounter, nullptr );
        _globalCounter = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::ExposurePass::_globalCounter" )
    }

    if ( _globalCounterMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _globalCounterMemory._memory, _globalCounterMemory._offset );
        _globalCounterMemory._memory = VK_NULL_HANDLE;
        _globalCounterMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_globalCounterMemory" )
    }

    if ( _exposureBarrier.buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _exposureBarrier.buffer, nullptr );
        _exposureBarrier.buffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::ExposurePass::_exposure" )
    }

    if ( _exposureMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _exposureMemory._memory, _exposureMemory._offset );
        _exposureMemory._memory = VK_NULL_HANDLE;
        _exposureMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_exposureMemory" )
    }

    if ( _luma != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _luma, nullptr );
        _luma = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::ExposurePass::_luma" )
    }

    if ( _lumaMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _lumaMemory._memory, _lumaMemory._offset );
        _lumaMemory._memory = VK_NULL_HANDLE;
        _lumaMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_lumaMemory" )
    }

    FreeTargetResources ( renderer, device );

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::ExposurePass::_descriptorPool" )
    }

    _layout.Destroy ( device );
    FreeTransferBuffer ( renderer, device );
}

bool ExposurePass::SetTarget ( android_vulkan::Renderer &renderer, android_vulkan::Texture2D const &hdrImage ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    VkExtent2D mipResolution;
    ExposureProgram::SpecializationInfo specData {};
    ExposureProgram::GetMetaInfo ( _dispatch, mipResolution, specData, hdrImage.GetResolution () );

    return
        UpdateMipCount ( renderer,
            device,
            static_cast<uint32_t> ( android_vulkan::Texture2D::CountMipLevels ( mipResolution ) ),
            specData
        ) &&

        CreateSyncMip5 ( renderer, device, specData._mip5Resolution ) &&
        BindTargetToDescriptorSet ( device, hdrImage );
}

VkExtent2D ExposurePass::AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept
{
    auto const adjust = [] ( uint32_t size ) constexpr -> uint32_t {
        constexpr uint32_t blockSize64Shift = 6U;
        constexpr uint32_t roundUP = ( 1U << blockSize64Shift ) - 1U;

        return std::max ( 512U,
            std::min ( 4095U, std::max ( 1U, ( size + roundUP ) >> blockSize64Shift ) << blockSize64Shift )
        );
    };

    return VkExtent2D
    {
        .width = adjust ( desiredResolution.width ),
        .height = adjust ( desiredResolution.height )
    };
}

bool ExposurePass::CreateDescriptorSet ( VkDevice device ) noexcept
{
    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1U,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 3U
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = 1U,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::ExposurePass::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::ExposurePass::_descriptorPool" )

    if ( !_layout.Init ( device ) )
        return false;

    VkDescriptorSetLayout layout = _layout.GetLayout ();

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &layout
    };

    return android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "pbr::ExposurePass::CreateDescriptorSet",
        "Can't create descriptor set"
    );
}

bool ExposurePass::CreateExposureResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    _exposureBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = static_cast<VkDeviceSize> ( sizeof ( float ) )
    };

    constexpr VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( float ),
        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_exposureBarrier.buffer ),
        "pbr::ExposurePass::CreateExposureResources",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::ExposurePass::_exposure" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _exposureBarrier.buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _exposureMemory._memory,
        _exposureMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate memory (pbr::ExposurePass::CreateExposureResources)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_exposureMemory" )

    return android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _exposureBarrier.buffer, _exposureMemory._memory, _exposureMemory._offset ),
        "pbr::ExposurePass::CreateExposureResources",
        "Can't memory"
    );
}

bool ExposurePass::CreateGlobalCounter ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    constexpr VkBufferUsageFlags usageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( uint32_t ),
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_globalCounter ),
        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't create global counter buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::ExposurePass::_globalCounter" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _globalCounter, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _globalCounterMemory._memory,
        _globalCounterMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate global counter memory (pbr::ExposurePass::CreateGlobalCounter)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_globalCounterMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _globalCounter, _globalCounterMemory._memory, _globalCounterMemory._offset ),
        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't bind global counter memory"
    );

    if ( !result )
        return false;

    bufferInfo.usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_SRC_BIT );

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferGlobalCounter ),
        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't create transfer buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::ExposurePass::_transferGlobalCounter" )

    vkGetBufferMemoryRequirements ( device, _transferGlobalCounter, &memoryRequirements );

    constexpr VkMemoryPropertyFlags memoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    result = renderer.TryAllocateMemory ( _transferGlobalCounterMemory._memory,
        _transferGlobalCounterMemory._offset,
        memoryRequirements,
        memoryFlags,
        "Can't allocate transfer buffer memory (pbr::ExposurePass::CreateGlobalCounter)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_transferGlobalCounterMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device,
            _transferGlobalCounter,
            _transferGlobalCounterMemory._memory,
            _transferGlobalCounterMemory._offset
        ),

        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't bind transfer memory"
    );

    if ( !result )
        return false;

    void* destination = nullptr;

    result = renderer.MapMemory ( destination,
        _transferGlobalCounterMemory._memory,
        _transferGlobalCounterMemory._offset,
        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't map transfer memory"
    );

    if ( !result )
        return false;

    auto* mappedBuffer = static_cast<uint32_t*> ( destination );
    std::memset ( mappedBuffer, 0, sizeof ( uint32_t ) );
    renderer.UnmapMemory ( _transferGlobalCounterMemory._memory );

    constexpr VkBufferCopy copyInfo
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = static_cast<VkDeviceSize> ( sizeof ( uint32_t ) )
    };

    vkCmdCopyBuffer ( _commandBuffer, _transferGlobalCounter, _globalCounter, 1U, &copyInfo );

    constexpr VkAccessFlags dstAccessFlags = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    VkBufferMemoryBarrier const barrierInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = dstAccessFlags,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _globalCounter,
        .offset = 0U,
        .size = copyInfo.size
    };

    vkCmdPipelineBarrier ( _commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    return true;
}

bool ExposurePass::CreateLumaResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    constexpr VkBufferUsageFlags usageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( float ),
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_luma ),
        "pbr::ExposurePass::CreateLumaResources",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::ExposurePass::_luma" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _luma, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _lumaMemory._memory,
        _lumaMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate memory (pbr::ExposurePass::CreateLumaResources)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_lumaMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _luma, _lumaMemory._memory, _lumaMemory._offset ),
        "pbr::ExposurePass::CreateLumaResources",
        "Can't memory"
    );

    if ( !result )
        return false;

    bufferInfo.usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_SRC_BIT );

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferLuma ),
        "pbr::ExposurePass::CreateLumaResources",
        "Can't create transfer buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::ExposurePass::_transferLuma" )

    vkGetBufferMemoryRequirements ( device, _transferLuma, &memoryRequirements );

    constexpr VkMemoryPropertyFlags memoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    result = renderer.TryAllocateMemory ( _transferLumaMemory._memory,
        _transferLumaMemory._offset,
        memoryRequirements,
        memoryFlags,
        "Can't allocate transfer buffer memory (pbr::ExposurePass::CreateLumaResources)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_transferLumaMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transferLuma, _transferLumaMemory._memory, _transferLumaMemory._offset ),
        "pbr::ExposurePass::CreateLumaResources",
        "Can't bind transfer memory"
    );

    if ( !result )
        return false;

    void* destination = nullptr;

    result = renderer.MapMemory ( destination,
        _transferLumaMemory._memory,
        _transferLumaMemory._offset,
        "pbr::ExposurePass::CreateLumaResources",
        "Can't map transfer memory"
    );

    if ( !result )
        return false;

    auto* mappedBuffer = static_cast<float*> ( destination );
    std::memset ( mappedBuffer, 0, sizeof ( float ) );
    renderer.UnmapMemory ( _transferLumaMemory._memory );

    constexpr VkBufferCopy copyInfo
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = static_cast<VkDeviceSize> ( sizeof ( float ) )
    };

    vkCmdCopyBuffer ( _commandBuffer, _transferLuma, _luma, 1U, &copyInfo );

    constexpr VkAccessFlags dstAccessFlags = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    VkBufferMemoryBarrier const barrierInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = dstAccessFlags,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _globalCounter,
        .offset = 0U,
        .size = copyInfo.size
    };

    vkCmdPipelineBarrier ( _commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    return true;
}

bool ExposurePass::CreateSyncMip5 ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkExtent2D resolution
) noexcept
{
    VkImageCreateInfo const imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R16_SFLOAT,

        .extent
        {
            .width = resolution.width,
            .height = resolution.height,
            .depth = 1U
        },

        .mipLevels = 1U,
        .arrayLayers = 1U,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = AV_VK_FLAG ( VK_IMAGE_USAGE_STORAGE_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateImage ( device, &imageInfo, nullptr, &_syncMip5 ),
        "pbr::ExposurePass::CreateSyncMip5",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "pbr::ExposurePass::_syncMip5" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _syncMip5, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _syncMip5Memory._memory,
        _syncMip5Memory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate image memory (pbr::ExposurePass::CreateSyncMip5)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_syncMip5Memory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindImageMemory ( device, _syncMip5, _syncMip5Memory._memory, _syncMip5Memory._offset ),
        "pbr::ExposurePass::CreateSyncMip5",
        "Can't bind image memory"
    );

    if ( !result )
        return false;

    VkImageViewCreateInfo const viewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = _syncMip5,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageInfo.format,

        .components
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkCreateImageView ( device, &viewInfo, nullptr, &_syncMip5View ),
        "pbr::ExposurePass::CreateMips",
        "Can't create view"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE_VIEW ( "pbr::ExposurePass::_syncMip5View" )
    _isNeedTransitLayout = true;
    return true;
}

bool ExposurePass::BindTargetToDescriptorSet ( VkDevice device, android_vulkan::Texture2D const &hdrImage ) noexcept
{
    VkDescriptorImageInfo const hrdImageInfo
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = hdrImage.GetImageView (),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo const syncMip5Info
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = _syncMip5View,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    VkDescriptorBufferInfo const exposureBufferInfo
    {
        .buffer = _exposureBarrier.buffer,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( float ) )
    };

    VkDescriptorBufferInfo const globalBufferInfo
    {
        .buffer = _globalCounter,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( uint32_t ) )
    };

    VkDescriptorBufferInfo const temporalLumaBufferInfo
    {
        .buffer = _luma,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( float ) )
    };

    VkWriteDescriptorSet const writes[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_HDR_IMAGE,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &hrdImageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_SYNC_MIP_5,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &syncMip5Info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_EXPOSURE,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &exposureBufferInfo,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_GLOBAL_ATOMIC,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &globalBufferInfo,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_TEMPORAL_LUMA,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &temporalLumaBufferInfo,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writes ) ), writes, 0U, nullptr );
    return true;
}

float ExposurePass::EyeAdaptationFactor ( float deltaTime ) const noexcept
{
    return 1.0F - std::exp ( -deltaTime * _eyeAdaptationSpeed );
}

void ExposurePass::FreeTargetResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    if ( _syncMip5View != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _syncMip5View, nullptr );
        _syncMip5View = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "pbr::ExposurePass::_syncMip5View" )
    }

    if ( _syncMip5 != VK_NULL_HANDLE )
    {
        vkDestroyImage ( device, _syncMip5, nullptr );
        _syncMip5 = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE ( "pbr::ExposurePass::_syncMip5" )
    }

    if ( _syncMip5Memory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _syncMip5Memory._memory, _syncMip5Memory._offset );
        _syncMip5Memory._memory = VK_NULL_HANDLE;
        _syncMip5Memory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_syncMip5Memory" )
    }

    _program.Destroy ( device );
}

void ExposurePass::FreeTransferBuffer ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    if ( _transferGlobalCounter != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transferGlobalCounter, nullptr );
        _transferGlobalCounter = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::ExposurePass::_transferGlobalCounter" )
    }

    if ( _transferGlobalCounterMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _transferGlobalCounterMemory._memory, _transferGlobalCounterMemory._offset );
        _transferGlobalCounterMemory._memory = VK_NULL_HANDLE;
        _transferGlobalCounterMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_transferGlobalCounterMemory" )
    }

    if ( _transferLuma != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transferLuma, nullptr );
        _transferLuma = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::ExposurePass::_transferLuma" )
    }

    if ( _transferLumaMemory._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _transferLumaMemory._memory, _transferLumaMemory._offset );
    _transferLumaMemory._memory = VK_NULL_HANDLE;
    _transferLumaMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::ExposurePass::_transferLumaMemory" )
}

bool ExposurePass::StartCommandBuffer ( VkCommandPool commandPool, VkDevice device ) noexcept
{
    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_commandBuffer ),
        "pbr::ExposurePass::StartCommandBuffer",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    return android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _commandBuffer, &beginInfo ),
        "pbr::ExposurePass::StartCommandBuffer",
        "Can't begin command buffer"
    );
}

bool ExposurePass::SubmitCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    bool const result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _commandBuffer ),
        "pbr::ExposurePass::SubmitCommandBuffer",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &_commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "pbr::ExposurePass::SubmitCommandBuffer",
        "Can't submit command"
    );
}

void ExposurePass::TransitSync5Mip ( VkCommandBuffer commandBuffer ) noexcept
{
    VkImageMemoryBarrier const barrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0U,
        .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = _syncMip5,

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrier
    );

    _isNeedTransitLayout = false;
}

bool ExposurePass::UpdateMipCount ( android_vulkan::Renderer &renderer,
    VkDevice device,
    uint32_t mipCount,
    ExposureProgram::SpecializationInfo const &specInfo
) noexcept
{
    if ( mipCount == _mipCount )
        return true;

    FreeTargetResources ( renderer, device );

    if ( !_program.Init ( renderer, &specInfo ) )
        return false;

    _mipCount = mipCount;
    return true;
}

float ExposurePass::ExposureValueToLuma ( float exposureValue ) noexcept
{
    // https://knarkowicz.wordpress.com/2016/01/09/automatic-exposure/
    return 0.125F * std::exp2 ( exposureValue );
}

} // namespace pbr
