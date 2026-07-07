#include <precompiled_headers.hpp>
#include <platform/windows/pbr/exposure_pass.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr float DEFAULT_EYE_ADAPTATION_SPEED = 1.0F;

constexpr float DEFAULT_EXPOSURE_COMPENSATION_EV = -10.0F;
constexpr float DEFAULT_MIN_LUMA_EV = -1.28F;
constexpr float DEFAULT_MAX_LUMA_EV = 15.0F;

constexpr size_t EXPOSURE_IDX = 0U;
constexpr size_t GLOBAL_COUNTER_IDX = 1U;
constexpr size_t LUMA_IDX = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void ExposurePass::Execute ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Exposure" )

    constexpr VkPipelineStageFlagBits2 const srcStage[] = {
        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
    };

    constexpr VkAccessFlags2 const srcAccess[] = { VK_ACCESS_2_SHADER_WRITE_BIT, VK_ACCESS_2_NONE };
    constexpr VkImageLayout const oldImageLayout[] = { VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_UNDEFINED };
    auto const selector = static_cast<size_t> ( _isNeedTransitLayout );

    _sync5Barrier.srcStageMask = srcStage[ selector ];
    _sync5Barrier.srcAccessMask = srcAccess[ selector ];
    _sync5Barrier.oldLayout = oldImageLayout[ selector ];

    _depInfo.imageMemoryBarrierCount = 1U;
    _depInfo.bufferMemoryBarrierCount = static_cast<uint32_t> ( std::size ( _barriers ) );
    _depInfo.pBufferMemoryBarriers = _barriers;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    _isNeedTransitLayout = false;

    _program.Bind ( commandBuffer );
    _exposureInfo._eyeAdaptation = EyeAdaptationFactor ( deltaTime );
    _program.SetPushConstants ( commandBuffer, &_exposureInfo );

    vkCmdDispatch ( commandBuffer, _dispatch.width, _dispatch.height, _dispatch.depth );

    _depInfo.imageMemoryBarrierCount = 0U;
    _depInfo.bufferMemoryBarrierCount = 1U;
    _depInfo.pBufferMemoryBarriers = &_exposureAfterBarrier;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );
}

void ExposurePass::FreeTransferResources ( VkDevice device, VkCommandPool commandPool ) noexcept
{
    vkFreeCommandBuffers ( device, commandPool, 1U, &_commandBuffer );
    _commandBuffer = VK_NULL_HANDLE;
}

uint32_t ExposurePass::GetExposure () const noexcept
{
    return _exposureInfo._exposure;
}

bool ExposurePass::Init ( android_vulkan::Renderer &renderer,
    ResourceHeap &resourceHeap,
    VkCommandPool commandPool
) noexcept
{
    VkBufferMemoryBarrier2 barriers[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = VK_WHOLE_SIZE
        },
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = VK_WHOLE_SIZE
        }
    };

    VkDevice device = renderer.GetDevice ();
    _eyeAdaptationSpeed = DEFAULT_EYE_ADAPTATION_SPEED;

    _exposureInfo._exposureCompensation = ExposureValueToLuma ( DEFAULT_EXPOSURE_COMPENSATION_EV );
    _exposureInfo._minLuma = ExposureValueToLuma ( DEFAULT_MIN_LUMA_EV );
    _exposureInfo._maxLuma = ExposureValueToLuma ( DEFAULT_MAX_LUMA_EV );

    bool const result = StartCommandBuffer ( commandPool, device ) &&
        CreateGlobalCounter ( renderer, device, resourceHeap, barriers[ 0U ].buffer ) &&
        CreateExposureResources ( renderer, device, resourceHeap ) &&
        CreateLumaResources ( renderer, device, resourceHeap, barriers[ 1U ].buffer );

    if ( !result ) [[unlikely]]
        return false;

    _depInfo.imageMemoryBarrierCount = 0U;
    _depInfo.bufferMemoryBarrierCount = static_cast<uint32_t> ( std::size ( barriers ) );
    _depInfo.pBufferMemoryBarriers = barriers;
    vkCmdPipelineBarrier2 ( _commandBuffer, &_depInfo );
    return SubmitCommandBuffer ( renderer );
}

void ExposurePass::Destroy ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( VkBuffer &buf = _barriers[ GLOBAL_COUNTER_IDX ].buffer; buf != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( device, std::exchange ( buf, VK_NULL_HANDLE ), nullptr );

    if ( uint32_t &idx = _exposureInfo._globalAtomic; idx ) [[likely]]
        resourceHeap.UnregisterResource ( std::exchange ( idx, 0U ) );

    if ( VkDeviceMemory &mem = _globalCounterMemory._memory; mem != VK_NULL_HANDLE ) [[likely]]
    {
        renderer.FreeMemory ( std::exchange ( mem, VK_NULL_HANDLE ),
            std::exchange ( _globalCounterMemory._offset, 0U )
        );
    }

    if ( uint32_t &idx = _exposureInfo._exposure; idx ) [[likely]]
        resourceHeap.UnregisterResource ( std::exchange ( idx, 0U ) );

    if ( VkBuffer &buf = _barriers[ EXPOSURE_IDX ].buffer; buf != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( device, std::exchange ( buf, VK_NULL_HANDLE ), nullptr );

    if ( VkDeviceMemory &mem = _exposureMemory._memory; mem != VK_NULL_HANDLE )
        renderer.FreeMemory ( std::exchange ( mem, VK_NULL_HANDLE ), std::exchange ( _exposureMemory._offset, 0U ) );

    if ( uint32_t &idx = _exposureInfo._temporalLuma; idx ) [[likely]]
        resourceHeap.UnregisterResource ( std::exchange ( idx, 0U ) );

    if ( VkBuffer &buf = _barriers[ LUMA_IDX ].buffer; buf != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( device, std::exchange ( buf, VK_NULL_HANDLE ), nullptr );

    if ( VkDeviceMemory &mem = _lumaMemory._memory; mem != VK_NULL_HANDLE ) [[likely]]
        renderer.FreeMemory ( std::exchange ( mem, VK_NULL_HANDLE ), std::exchange ( _lumaMemory._offset, 0U ) );

    FreeTargetResources ( renderer, device, resourceHeap );
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

bool ExposurePass::SetTarget ( android_vulkan::Renderer &renderer,
    ResourceHeap &resourceHeap,
    android_vulkan::Texture2D const &hdrImage,
    uint32_t hdrImageIndex
) noexcept
{
    _exposureInfo._hdrImage = hdrImageIndex;

    ExposureSpecialization const specData ( hdrImage.GetResolution () );
    _dispatch = specData._dispatch;
    return UpdateSyncMip5 ( renderer, resourceHeap, specData );
}

bool ExposurePass::CreateExposureResources ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap
) noexcept
{
    constexpr VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( float ),

        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ),

        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkBuffer &buffer = _exposureAfterBarrier.buffer;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "pbr::ExposurePass::CreateExposureResources",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "Exposure" )
    _barriers[ EXPOSURE_IDX ].buffer = buffer;

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _exposureMemory._memory,
            _exposureMemory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Can't allocate memory (pbr::ExposurePass::CreateExposureResources)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _exposureMemory._memory, _exposureMemory._offset ),
            "pbr::ExposurePass::CreateExposureResources",
            "Can't memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    auto const idx = resourceHeap.RegisterBuffer ( device, buffer, bufferInfo.size );

    if ( !idx ) [[unlikely]]
        return false;

    _exposureInfo._exposure = *idx;
    return true;
}

bool ExposurePass::CreateGlobalCounter ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap,
    VkBuffer &buffer
) noexcept
{
    constexpr VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( uint32_t ),

        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ),

        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "pbr::ExposurePass::CreateGlobalCounter",
        "Can't create global counter buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "Exposure SPD global counter" )
    _barriers[ GLOBAL_COUNTER_IDX ].buffer = buffer;

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _globalCounterMemory._memory,
            _globalCounterMemory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Can't allocate global counter memory (pbr::ExposurePass::CreateGlobalCounter)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _globalCounterMemory._memory, _globalCounterMemory._offset ),
            "pbr::ExposurePass::CreateGlobalCounter",
            "Can't bind global counter memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    auto const idx = resourceHeap.RegisterBuffer ( device, buffer, bufferInfo.size );

    if ( !idx ) [[unlikely]]
        return false;

    _exposureInfo._globalAtomic = *idx;
    vkCmdFillBuffer ( _commandBuffer, buffer, 0U, VK_WHOLE_SIZE, 0U );
    return true;
}

bool ExposurePass::CreateLumaResources ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap,
    VkBuffer &buffer
) noexcept
{
    constexpr VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( float ),

        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ),

        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "pbr::ExposurePass::CreateLumaResources",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "Luma" )
    _barriers[ LUMA_IDX ].buffer = buffer;

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _lumaMemory._memory,
            _lumaMemory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Can't allocate memory (pbr::ExposurePass::CreateLumaResources)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _lumaMemory._memory, _lumaMemory._offset ),
            "pbr::ExposurePass::CreateLumaResources",
            "Can't memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    auto const idx = resourceHeap.RegisterBuffer ( device, buffer, bufferInfo.size );

    if ( !idx ) [[unlikely]]
        return false;

    _exposureInfo._temporalLuma = *idx;
    vkCmdFillBuffer ( _commandBuffer, buffer, 0U, VK_WHOLE_SIZE, 0U );
    return true;
}

float ExposurePass::EyeAdaptationFactor ( float deltaTime ) const noexcept
{
    return 1.0F - std::exp ( -deltaTime * _eyeAdaptationSpeed );
}

void ExposurePass::FreeTargetResources ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap
) noexcept
{
    if ( uint32_t &idx = _exposureInfo._syncMip5; idx ) [[likely]]
        resourceHeap.UnregisterResource ( std::exchange ( idx, 0U ) );

    if ( _syncMip5View != VK_NULL_HANDLE ) [[likely]]
        vkDestroyImageView ( device, std::exchange ( _syncMip5View, VK_NULL_HANDLE ), nullptr );

    if ( _sync5Barrier.image != VK_NULL_HANDLE ) [[likely]]
        vkDestroyImage ( device, std::exchange ( _sync5Barrier.image, VK_NULL_HANDLE ), nullptr );

    if ( _syncMip5Memory._memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    renderer.FreeMemory ( std::exchange ( _syncMip5Memory._memory, VK_NULL_HANDLE ),
        std::exchange ( _syncMip5Memory._offset, 0U )
    );
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

bool ExposurePass::UpdateSyncMip5 ( android_vulkan::Renderer &renderer,
    ResourceHeap &resourceHeap,
    ExposureSpecialization const &specInfo
) noexcept
{
    VkExtent2D const &mip5 = specInfo._mip5Resolution;

    if ( ( mip5.width == _mip5resolution.width ) & ( mip5.height == _mip5resolution.height ) ) [[unlikely]]
        return true;

    VkDevice device = renderer.GetDevice ();
    FreeTargetResources ( renderer, device, resourceHeap );

    VkImageCreateInfo const imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R16_SFLOAT,

        .extent
        {
            .width = mip5.width,
            .height = mip5.height,
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

    VkImage &syncMip5 = _sync5Barrier.image;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateImage ( device, &imageInfo, nullptr, &syncMip5 ),
        "pbr::ExposurePass::UpdateSyncMip5",
        "Can't create image"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, syncMip5, VK_OBJECT_TYPE_IMAGE, "Exposure mip #5" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, syncMip5, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _syncMip5Memory._memory,
            _syncMip5Memory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Can't allocate image memory (pbr::ExposurePass::UpdateSyncMip5)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindImageMemory ( device, syncMip5, _syncMip5Memory._memory, _syncMip5Memory._offset ),
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
        .image = syncMip5,
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

    auto const idx = resourceHeap.RegisterStorageImage ( device, _syncMip5View );

    if ( !idx ) [[unlikely]]
        return false;

    _exposureInfo._syncMip5 = *idx;

    AV_SET_VULKAN_OBJECT_NAME ( device, _syncMip5View, VK_OBJECT_TYPE_IMAGE_VIEW, "Exposure mip #5" )
    _isNeedTransitLayout = true;
    _mip5resolution = mip5;

    _program.Destroy ( device );
    return _program.Init ( device, &specInfo );
}

float ExposurePass::ExposureValueToLuma ( float exposureValue ) noexcept
{
    // https://knarkowicz.wordpress.com/2016/01/09/automatic-exposure/
    return 0.125F * std::exp2 ( exposureValue );
}

} // namespace pbr
