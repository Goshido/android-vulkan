#include <pbr/average_brightness.inc>
#include <pbr/average_brightness_pass.hpp>
#include <av_assert.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

//----------------------------------------------------------------------------------------------------------------------

void AverageBrightnessPass::Execute ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( _isNeedTransitLayout )
        TransitSync5Mip ( commandBuffer );

    _program.Bind ( commandBuffer );
    _program.SetDescriptorSet ( commandBuffer, _descriptorSet );
    vkCmdDispatch ( commandBuffer, _dispatch.width, _dispatch.height, _dispatch.depth );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        1U,
        &_brightnessBarrier,
        0U,
        nullptr
    );
}

void AverageBrightnessPass::FreeTransferResources ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    vkFreeCommandBuffers ( device, commandPool, 1U, &_commandBuffer );
    _commandBuffer = VK_NULL_HANDLE;
    FreeTransferBuffer ( renderer, device );
}

bool AverageBrightnessPass::Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    return CreateGlobalCounter ( renderer, device, commandPool ) &&
        CreateDescriptorPool ( device ) &&
        CreateBrightnessResources ( renderer, device ) &&
        _layout.Init ( device );
}

void AverageBrightnessPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _globalCounter != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _globalCounter, nullptr );
        _globalCounter = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::AverageBrightnessPass::_globalCounter" )
    }

    if ( _globalCounterMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _globalCounterMemory._memory, _globalCounterMemory._offset );
        _globalCounterMemory._memory = VK_NULL_HANDLE;
        _globalCounterMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_globalCounterMemory" )
    }

    if ( _brightnessBarrier.buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _brightnessBarrier.buffer, nullptr );
        _brightnessBarrier.buffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::AverageBrightnessPass::_brightness" )
    }

    if ( _brightnessMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _brightnessMemory._memory, _brightnessMemory._offset );
        _brightnessMemory._memory = VK_NULL_HANDLE;
        _brightnessMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_brightnessMemory" )
    }

    FreeTargetResources ( renderer, device );

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::AverageBrightnessPass::_descriptorPool" )
    }

    _layout.Destroy ( device );
    FreeTransferBuffer ( renderer, device );
}

bool AverageBrightnessPass::SetTarget ( android_vulkan::Renderer &renderer,
    android_vulkan::Texture2D const &hdrImage
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    VkExtent2D mipResolution;
    AverageBrightnessProgram::SpecializationInfo specData {};
    AverageBrightnessProgram::GetMetaInfo ( _dispatch, mipResolution, specData, hdrImage.GetResolution () );

    return
        UpdateMipCount ( renderer,
            device,
            static_cast<uint32_t> ( android_vulkan::Texture2D::CountMipLevels ( mipResolution ) ),
            specData
        ) &&

        CreateSyncMip5 ( renderer, device, specData._mip5Resolution ) &&
        BindTargetToDescriptorSet ( device, hdrImage );
}

VkExtent2D AverageBrightnessPass::AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept
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

bool AverageBrightnessPass::CreateBrightnessResources ( android_vulkan::Renderer &renderer,
    VkDevice device
) noexcept
{
    _brightnessBarrier =
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
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_brightnessBarrier.buffer ),
        "pbr::AverageBrightnessPass::CreateBrightnessResources",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::AverageBrightnessPass::_brightness" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _brightnessBarrier.buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _brightnessMemory._memory,
        _brightnessMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate memory (pbr::AverageBrightnessPass::CreateBrightnessResources)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_brightnessMemory" )

    return android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _brightnessBarrier.buffer, _brightnessMemory._memory, _brightnessMemory._offset ),
        "pbr::AverageBrightnessPass::CreateBrightnessResources",
        "Can't memory"
    );
}

bool AverageBrightnessPass::CreateDescriptorPool ( VkDevice device ) noexcept
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
            .descriptorCount = 2U
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1U,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::AverageBrightnessPass::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::AverageBrightnessPass::_descriptorPool" )
    return true;
}

bool AverageBrightnessPass::CreateGlobalCounter ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkCommandPool commandPool
) noexcept
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
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't create global counter buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::AverageBrightnessPass::_globalCounter" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _globalCounter, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _globalCounterMemory._memory,
        _globalCounterMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate global counter memory (pbr::AverageBrightnessPass::CreateGlobalCounter)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_globalCounterMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _globalCounter, _globalCounterMemory._memory, _globalCounterMemory._offset ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't bind global counter memory"
    );

    if ( !result )
        return false;

    bufferInfo.usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_SRC_BIT );

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferBuffer ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't create transfer buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::AverageBrightnessPass::_transferBuffer" )

    vkGetBufferMemoryRequirements ( device, _transferBuffer, &memoryRequirements );

    constexpr VkMemoryPropertyFlags const memoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    result = renderer.TryAllocateMemory ( _transferBufferMemory._memory,
        _transferBufferMemory._offset,
        memoryRequirements,
        memoryFlags,
        "Can't allocate transfer buffer memory (pbr::AverageBrightnessPass::CreateGlobalCounter)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_transferBufferMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transferBuffer, _transferBufferMemory._memory, _transferBufferMemory._offset ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't bind transfer memory"
    );

    if ( !result )
        return false;

    void* destination = nullptr;

    result = renderer.MapMemory ( destination,
        _transferBufferMemory._memory,
        _transferBufferMemory._offset,
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't map transfer memory"
    );

    if ( !result )
        return false;

    auto* mappedBuffer = static_cast<uint8_t*> ( destination );
    std::memset ( mappedBuffer, 0, sizeof ( uint32_t ) );
    renderer.UnmapMemory ( _transferBufferMemory._memory );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_commandBuffer ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
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

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _commandBuffer, &beginInfo ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    constexpr VkBufferCopy copyInfo
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = static_cast<VkDeviceSize> ( sizeof ( uint32_t ) )
    };

    vkCmdCopyBuffer ( _commandBuffer, _transferBuffer, _globalCounter, 1U, &copyInfo );

    constexpr VkAccessFlags dstAccessFlags = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    VkBufferMemoryBarrier const barrierInfo =
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
        0U,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _commandBuffer ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
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
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't submit command"
    );
}

bool AverageBrightnessPass::CreateSyncMip5 ( android_vulkan::Renderer &renderer,
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
        "pbr::AverageBrightnessPass::CreateSyncMip5",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "pbr::AverageBrightnessPass::_syncMip5" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _syncMip5, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _syncMip5Memory._memory,
        _syncMip5Memory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate image memory (pbr::AverageBrightnessPass::CreateSyncMip5)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_syncMip5Memory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindImageMemory ( device, _syncMip5, _syncMip5Memory._memory, _syncMip5Memory._offset ),
        "pbr::AverageBrightnessPass::CreateSyncMip5",
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
        "pbr::AverageBrightnessPass::CreateMips",
        "Can't create view"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE_VIEW ( "pbr::AverageBrightnessPass::_syncMip5View" )
    _isNeedTransitLayout = true;
    return true;
}

bool AverageBrightnessPass::BindTargetToDescriptorSet ( VkDevice device,
    android_vulkan::Texture2D const &hdrImage
) noexcept
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

    VkDescriptorBufferInfo const counterCounterBufferInfo
    {
        .buffer = _globalCounter,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( uint32_t ) )
    };

    VkDescriptorBufferInfo const brightnessBufferInfo
    {
        .buffer = _brightnessBarrier.buffer,
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
            .dstBinding = BIND_GLOBAL_ATOMIC,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &counterCounterBufferInfo,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_BRIGHTNESS,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &brightnessBufferInfo,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writes ) ), writes, 0U, nullptr );
    return true;
}

void AverageBrightnessPass::FreeTargetResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    if ( _syncMip5View != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _syncMip5View, nullptr );
        _syncMip5View = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "pbr::AverageBrightnessPass::_syncMip5View" )
    }

    if ( _syncMip5 != VK_NULL_HANDLE )
    {
        vkDestroyImage ( device, _syncMip5, nullptr );
        _syncMip5 = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE ( "pbr::AverageBrightnessPass::_syncMip5" )
    }

    if ( _syncMip5Memory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _syncMip5Memory._memory, _syncMip5Memory._offset );
        _syncMip5Memory._memory = VK_NULL_HANDLE;
        _syncMip5Memory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_syncMip5Memory" )
    }

    _program.Destroy ( device );
}

void AverageBrightnessPass::FreeTransferBuffer ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    if ( _transferBuffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transferBuffer, nullptr );
        _transferBuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::AverageBrightnessPass::_transferBuffer" )
    }

    if ( _transferBufferMemory._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _transferBufferMemory._memory, _transferBufferMemory._offset );
    _transferBufferMemory._memory = VK_NULL_HANDLE;
    _transferBufferMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_transferBufferMemory" )
}

void AverageBrightnessPass::TransitSync5Mip ( VkCommandBuffer commandBuffer ) noexcept
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

bool AverageBrightnessPass::UpdateMipCount ( android_vulkan::Renderer &renderer,
    VkDevice device,
    uint32_t mipCount,
    AverageBrightnessProgram::SpecializationInfo const &specInfo
) noexcept
{
    if ( mipCount == _mipCount )
        return true;

    FreeTargetResources ( renderer, device );

    if ( _descriptorSet != VK_NULL_HANDLE )
    {
        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkFreeDescriptorSets ( device, _descriptorPool, 1U, &_descriptorSet ),
            "pbr::AverageBrightnessPass::UpdateMipCount",
            "Can't free descriptor set"
        );

        if ( !result )
        {
            return false;
        }
    }

    if ( !_program.Init ( renderer, &specInfo ) )
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

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "pbr::AverageBrightnessPass::UpdateMipCount",
        "Can't create descriptor set"
    );

    if ( !result )
        return false;

    _mipCount = mipCount;
    return true;
}

} // namespace pbr
