#include <precompiled_headers.hpp>
#include <pbr/exposure.inc>
#include <pbr/exposure_pass.hpp>
#include <av_assert.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr float DEFAULT_EYE_ADAPTATION_SPEED = 1.0F;

constexpr float DEFAULT_EXPOSURE_COMPENSATION_EV = -10.0F;
constexpr float DEFAULT_MIN_LUMA_EV = -1.28F;
constexpr float DEFAULT_MAX_LUMA_EV = 15.0F;

constexpr size_t GLOBAL_COUNTER_IDX = 0U;
constexpr size_t LUMA_IDX = 1U;

} // end of anonymous

//----------------------------------------------------------------------------------------------------------------------

void ExposurePass::Execute ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Exposure" )
    SyncBefore ( commandBuffer );

    _program.Bind ( commandBuffer );

    _exposureInfo._eyeAdaptation = EyeAdaptationFactor ( deltaTime );
    _program.SetPushConstants ( commandBuffer, &_exposureInfo );

    _program.SetDescriptorSet ( commandBuffer, _descriptorSet );

    vkCmdDispatch ( commandBuffer, _dispatch.width, _dispatch.height, _dispatch.depth );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &_exposureAfterBarrier,
        0U,
        nullptr
    );
}

void ExposurePass::FreeTransferResources ( VkDevice device, VkCommandPool commandPool ) noexcept
{
    vkFreeCommandBuffers ( device, commandPool, 1U, &_commandBuffer );
    _commandBuffer = VK_NULL_HANDLE;
}

VkBuffer ExposurePass::GetExposure () const noexcept
{
    return _exposureBeforeBarrier.buffer;
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
    VkBuffer &globalCounter = _computeOnlyBarriers[ GLOBAL_COUNTER_IDX ].buffer;

    if ( globalCounter != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyBuffer ( device, globalCounter, nullptr );
        globalCounter = VK_NULL_HANDLE;
    }

    if ( _globalCounterMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _globalCounterMemory._memory, _globalCounterMemory._offset );
        _globalCounterMemory._memory = VK_NULL_HANDLE;
        _globalCounterMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    if ( _exposureBeforeBarrier.buffer != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyBuffer ( device, _exposureBeforeBarrier.buffer, nullptr );
        _exposureBeforeBarrier.buffer = VK_NULL_HANDLE;
    }

    if ( _exposureMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _exposureMemory._memory, _exposureMemory._offset );
        _exposureMemory._memory = VK_NULL_HANDLE;
        _exposureMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    VkBuffer &luma = _computeOnlyBarriers[ LUMA_IDX ].buffer;

    if ( luma != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyBuffer ( device, luma, nullptr );
        luma = VK_NULL_HANDLE;
    }

    if ( _lumaMemory._memory != VK_NULL_HANDLE ) [[likely]]
    {
        renderer.FreeMemory ( _lumaMemory._memory, _lumaMemory._offset );
        _lumaMemory._memory = VK_NULL_HANDLE;
        _lumaMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    FreeTargetResources ( renderer, device );

    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    _layout.Destroy ( device );
    _program.Destroy ( device );
}

void ExposurePass::SetMaximumBrightness ( float exposureValue ) noexcept
{
    _exposureInfo._maxLuma = ExposureValueToLuma ( exposureValue );
}

void ExposurePass::SetMinimumBrightness ( float exposureValue ) noexcept
{
    _exposureInfo._minLuma = ExposureValueToLuma ( exposureValue );
}

void ExposurePass::SetExposureCompensation ( float exposureValue ) noexcept
{
    _exposureInfo._exposureCompensation = ExposureValueToLuma ( exposureValue );
}

void ExposurePass::SetEyeAdaptationSpeed ( float speed ) noexcept
{
    _eyeAdaptationSpeed = speed;
}

bool ExposurePass::SetTarget ( android_vulkan::Renderer &renderer, android_vulkan::Texture2D const &hdrImage ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    VkExtent2D mipResolution;
    ExposureProgram::SpecializationInfo specData {};
    ExposureProgram::GetMetaInfo ( _dispatch, mipResolution, specData, hdrImage.GetResolution () );

    bool const result = UpdateSyncMip5 ( renderer, device, specData._mip5Resolution ) &&

        UpdateMipCount ( renderer,
            device,
            static_cast<uint32_t> ( android_vulkan::Texture2D::CountMipLevels ( mipResolution ) ),
            specData
        );

    if ( !result ) [[unlikely]]
        return false;

    BindTargetToDescriptorSet ( device, hdrImage );
    return true;
}

VkExtent2D ExposurePass::AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept
{
    constexpr auto adjust = [] ( uint32_t size ) constexpr -> uint32_t {
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

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::ExposurePass::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Exposure" )

    if ( !_layout.Init ( device ) ) [[unlikely]]
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

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "pbr::ExposurePass::CreateDescriptorSet",
        "Can't create descriptor set"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Exposure" )
    return true;
}

bool ExposurePass::CreateExposureResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    _exposureBeforeBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_UNIFORM_READ_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = VK_WHOLE_SIZE
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
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_exposureBeforeBarrier.buffer ),
        "pbr::ExposurePass::CreateExposureResources",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _exposureBeforeBarrier.buffer, VK_OBJECT_TYPE_BUFFER, "Exposure" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _exposureBeforeBarrier.buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _exposureMemory._memory,
        _exposureMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate memory (pbr::ExposurePass::CreateExposureResources)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _exposureBeforeBarrier.buffer, _exposureMemory._memory, _exposureMemory._offset ),
        "pbr::ExposurePass::CreateExposureResources",
        "Can't memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    _exposureAfterBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _exposureBeforeBarrier.buffer,
        .offset = 0U,
        .size = VK_WHOLE_SIZE
    };

    return true;
}

bool ExposurePass::CreateGlobalCounter ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    VkBufferMemoryBarrier &barrier = _computeOnlyBarriers[ GLOBAL_COUNTER_IDX ];

    barrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),
        .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = VK_WHOLE_SIZE
    };

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( uint32_t ),
        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkBuffer &globalCounter = barrier.buffer;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &globalCounter ),
        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't create global counter buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, globalCounter, VK_OBJECT_TYPE_BUFFER, "Exposure SPD global counter" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, globalCounter, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _globalCounterMemory._memory,
        _globalCounterMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate global counter memory (pbr::ExposurePass::CreateGlobalCounter)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, globalCounter, _globalCounterMemory._memory, _globalCounterMemory._offset ),
        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't bind global counter memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    vkCmdFillBuffer ( _commandBuffer, globalCounter, 0U, VK_WHOLE_SIZE, 0U );

    VkBufferMemoryBarrier const barrierInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = globalCounter,
        .offset = 0U,
        .size = VK_WHOLE_SIZE
    };

    vkCmdPipelineBarrier ( _commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
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
    VkBufferMemoryBarrier &barrier = _computeOnlyBarriers[ LUMA_IDX ];

    barrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),
        .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = VK_WHOLE_SIZE
    };

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( float ),
        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkBuffer &luma = barrier.buffer;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &luma ),
        "pbr::ExposurePass::CreateLumaResources",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, luma, VK_OBJECT_TYPE_BUFFER, "Luma" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, luma, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _lumaMemory._memory,
        _lumaMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate memory (pbr::ExposurePass::CreateLumaResources)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, luma, _lumaMemory._memory, _lumaMemory._offset ),
        "pbr::ExposurePass::CreateLumaResources",
        "Can't memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    vkCmdFillBuffer ( _commandBuffer, luma, 0U, VK_WHOLE_SIZE, 0U );

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
        .buffer = luma,
        .offset = 0U,
        .size = VK_WHOLE_SIZE
    };

    vkCmdPipelineBarrier ( _commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    return true;
}

void ExposurePass::BindTargetToDescriptorSet ( VkDevice device, android_vulkan::Texture2D const &hdrImage ) noexcept
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
        .buffer = _exposureBeforeBarrier.buffer,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( float ) )
    };

    VkDescriptorBufferInfo const globalBufferInfo
    {
        .buffer = _computeOnlyBarriers[ GLOBAL_COUNTER_IDX ].buffer,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( uint32_t ) )
    };

    VkDescriptorBufferInfo const temporalLumaBufferInfo
    {
        .buffer = _computeOnlyBarriers[ LUMA_IDX ].buffer,
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
}

float ExposurePass::EyeAdaptationFactor ( float deltaTime ) const noexcept
{
    return 1.0F - std::exp ( -deltaTime * _eyeAdaptationSpeed );
}

void ExposurePass::FreeTargetResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    if ( _syncMip5View != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyImageView ( device, _syncMip5View, nullptr );
        _syncMip5View = VK_NULL_HANDLE;
    }

    if ( _syncMip5 != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyImage ( device, _syncMip5, nullptr );
        _syncMip5 = VK_NULL_HANDLE;
    }

    if ( _syncMip5Memory._memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    renderer.FreeMemory ( _syncMip5Memory._memory, _syncMip5Memory._offset );
    _syncMip5Memory._memory = VK_NULL_HANDLE;
    _syncMip5Memory._offset = std::numeric_limits<VkDeviceSize>::max ();
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

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "Exposure pass resource init" )

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

    if ( !result ) [[unlikely]]
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

void ExposurePass::SyncBefore ( VkCommandBuffer commandBuffer ) noexcept
{
    constexpr VkAccessFlags const srcAccess[] = { VK_ACCESS_SHADER_WRITE_BIT, 0U };
    constexpr VkImageLayout const oldImageLayout[] = { VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_UNDEFINED };
    auto const selector = static_cast<size_t> ( _isNeedTransitLayout );

    VkImageMemoryBarrier const barrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = srcAccess[ selector ],
        .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .oldLayout = oldImageLayout[ selector ],
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

    constexpr VkPipelineStageFlagBits const srcStage[] = {
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    };

    vkCmdPipelineBarrier ( commandBuffer,
        srcStage[ selector ],
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrier
    );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( std::size ( _computeOnlyBarriers ) ),
        _computeOnlyBarriers,
        0U,
        nullptr
    );

    constexpr auto stages = static_cast<VkPipelineStageFlags> ( AV_VK_FLAG ( VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT )
    );

    vkCmdPipelineBarrier ( commandBuffer,
        stages,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &_exposureBeforeBarrier,
        0U,
        nullptr
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

    _program.Destroy ( device );

    if ( !_program.Init ( renderer, &specInfo ) ) [[unlikely]]
        return false;

    _mipCount = mipCount;
    return true;
}

bool ExposurePass::UpdateSyncMip5 ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkExtent2D const &resolution
) noexcept
{
    if ( resolution.width == _mip5resolution.width && resolution.height == _mip5resolution.height ) [[unlikely]]
        return true;

    FreeTargetResources ( renderer, device );

    VkImageCreateInfo const imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .imageType = VK_IMAGE_TYPE_2D,

        // [2025/06/28] Can't use float16_t format because of DXC issue.
        // See https://github.com/microsoft/DirectXShaderCompiler/issues/7595
        .format = VK_FORMAT_R32_SFLOAT,

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
        "pbr::ExposurePass::UpdateSyncMip5",
        "Can't create image"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _syncMip5, VK_OBJECT_TYPE_IMAGE, "Exposure mip #5" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _syncMip5, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _syncMip5Memory._memory,
        _syncMip5Memory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate image memory (pbr::ExposurePass::UpdateSyncMip5)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindImageMemory ( device, _syncMip5, _syncMip5Memory._memory, _syncMip5Memory._offset ),
        "pbr::ExposurePass::UpdateSyncMip5",
        "Can't bind image memory"
    );

    if ( !result ) [[unlikely]]
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
        "pbr::ExposurePass::UpdateSyncMip5",
        "Can't create view"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _syncMip5View, VK_OBJECT_TYPE_IMAGE_VIEW, "Exposure mip #5" )
    _isNeedTransitLayout = true;
    _mip5resolution = resolution;
    return true;
}

float ExposurePass::ExposureValueToLuma ( float exposureValue ) noexcept
{
    // https://knarkowicz.wordpress.com/2016/01/09/automatic-exposure/
    return 0.125F * std::exp2 ( exposureValue );
}

} // namespace pbr
