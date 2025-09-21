// FUCK - windows and android separation

#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/exposure.inc>
#include <platform/windows/pbr/exposure_pass.hpp>
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::windows {

namespace {

constexpr float DEFAULT_EYE_ADAPTATION_SPEED = 1.0F;

constexpr float DEFAULT_EXPOSURE_COMPENSATION_EV = -10.0F;
constexpr float DEFAULT_MIN_LUMA_EV = -1.28F;
constexpr float DEFAULT_MAX_LUMA_EV = 15.0F;

constexpr size_t GLOBAL_COUNTER_IDX = 0U;
constexpr size_t LUMA_IDX = 1U;

} // end of anonymous

//----------------------------------------------------------------------------------------------------------------------

void ExposurePass::Execute ( VkCommandBuffer commandBuffer, float deltaTime, ResourceHeap &resourceHeap ) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Exposure" )
    SyncBefore ( commandBuffer );

    _program.Bind ( commandBuffer );
    resourceHeap.Bind ( commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _program.GetPipelineLayout () );

    _exposureInfo._eyeAdaptation = EyeAdaptationFactor ( deltaTime );
    _program.SetPushConstants ( commandBuffer, &_exposureInfo );

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

uint32_t ExposurePass::GetExposure () const noexcept
{
    return _exposureInfo._exposure;
}

bool ExposurePass::Init ( android_vulkan::Renderer &renderer,
    ResourceHeap &resourceHeap,
    VkCommandPool commandPool
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _eyeAdaptationSpeed = DEFAULT_EYE_ADAPTATION_SPEED;

    _exposureInfo._exposureCompensation = ExposureValueToLuma ( DEFAULT_EXPOSURE_COMPENSATION_EV );
    _exposureInfo._minLuma = ExposureValueToLuma ( DEFAULT_MIN_LUMA_EV );
    _exposureInfo._maxLuma = ExposureValueToLuma ( DEFAULT_MAX_LUMA_EV );

    return StartCommandBuffer ( commandPool, device ) &&
        CreateGlobalCounter ( renderer, device, resourceHeap ) &&
        CreateExposureResources ( renderer, device, resourceHeap ) &&
        CreateLumaResources ( renderer, device, resourceHeap ) &&
        SubmitCommandBuffer ( renderer );
}

void ExposurePass::Destroy ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( VkBuffer &buf = _computeOnlyBarriers[ GLOBAL_COUNTER_IDX ].buffer; buf != VK_NULL_HANDLE ) [[likely]]
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

    if ( VkBuffer &buf = _exposureBeforeBarrier.buffer; buf != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( device, std::exchange ( buf, VK_NULL_HANDLE ), nullptr );

    if ( VkDeviceMemory &mem = _exposureMemory._memory; mem != VK_NULL_HANDLE )
        renderer.FreeMemory ( std::exchange ( mem, VK_NULL_HANDLE ), std::exchange ( _exposureMemory._offset, 0U ) );

    if ( uint32_t &idx = _exposureInfo._temporalLuma; idx ) [[likely]]
        resourceHeap.UnregisterResource ( std::exchange ( idx, 0U ) );

    if ( VkBuffer &buf = _computeOnlyBarriers[ LUMA_IDX ].buffer; buf != VK_NULL_HANDLE ) [[likely]]
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
    VkDevice device = renderer.GetDevice ();
    _exposureInfo._hdrImage = hdrImageIndex;

    VkExtent2D mipResolution;
    ExposureSpecialization const specData ( _dispatch, mipResolution, hdrImage.GetResolution () );

    return UpdateSyncMip5 ( renderer, device, resourceHeap, specData._mip5Resolution ) &&

        UpdateMipCount ( device,
            static_cast<uint32_t> ( android_vulkan::Texture2D::CountMipLevels ( mipResolution ) ),
            specData
        );
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

bool ExposurePass::CreateExposureResources ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap
) noexcept
{
    _exposureBeforeBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),
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

        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ),

        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkBuffer &buffer = _exposureBeforeBarrier.buffer;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::CreateExposureResources",

        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "Exposure" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _exposureMemory._memory,
            _exposureMemory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,

            // FUCK - remove namespace
            "Can't allocate memory (pbr::windows::ExposurePass::CreateExposureResources)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _exposureMemory._memory, _exposureMemory._offset ),

            // FUCK - remove namespace
            "pbr::windows::ExposurePass::CreateExposureResources",

            "Can't memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    auto const idx = resourceHeap.RegisterBuffer ( device, buffer, bufferInfo.size );

    if ( !idx ) [[unlikely]]
        return false;

    _exposureInfo._exposure = *idx;

    _exposureAfterBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = 0U,
        .size = VK_WHOLE_SIZE
    };

    return true;
}

bool ExposurePass::CreateGlobalCounter ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap
) noexcept
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

    VkBuffer &buffer = barrier.buffer;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::CreateGlobalCounter",

        "Can't create global counter buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "Exposure SPD global counter" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _globalCounterMemory._memory,
            _globalCounterMemory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,

            // FUCK - remove namespace
            "Can't allocate global counter memory (pbr::windows::ExposurePass::CreateGlobalCounter)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _globalCounterMemory._memory, _globalCounterMemory._offset ),

            // FUCK - remove namespace
            "pbr::windows::ExposurePass::CreateGlobalCounter",

            "Can't bind global counter memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    auto const idx = resourceHeap.RegisterBuffer ( device, buffer, bufferInfo.size );

    if ( !idx ) [[unlikely]]
        return false;

    _exposureInfo._globalAtomic = *idx;
    vkCmdFillBuffer ( _commandBuffer, buffer, 0U, VK_WHOLE_SIZE, 0U );

    VkBufferMemoryBarrier const barrierInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
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

bool ExposurePass::CreateLumaResources ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap
) noexcept
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

    VkBuffer &buffer = barrier.buffer;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::CreateLumaResources",

        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "Luma" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _lumaMemory._memory,
            _lumaMemory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,

            // FUCK - remove namespace
            "Can't allocate memory (pbr::windows::ExposurePass::CreateLumaResources)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _lumaMemory._memory, _lumaMemory._offset ),

            // FUCK - remove namespace
            "pbr::windows::ExposurePass::CreateLumaResources",

            "Can't memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    auto const idx = resourceHeap.RegisterBuffer ( device, buffer, bufferInfo.size );

    if ( !idx ) [[unlikely]]
        return false;

    _exposureInfo._temporalLuma = *idx;
    vkCmdFillBuffer ( _commandBuffer, buffer, 0U, VK_WHOLE_SIZE, 0U );

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
        .buffer = buffer,
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

    if ( _syncMip5 != VK_NULL_HANDLE ) [[likely]]
        vkDestroyImage ( device, std::exchange ( _syncMip5, VK_NULL_HANDLE ), nullptr );

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

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::StartCommandBuffer",

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

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::StartCommandBuffer",

        "Can't begin command buffer"
    );
}

bool ExposurePass::SubmitCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    bool const result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _commandBuffer ),
        "pbr::windows::ExposurePass::SubmitCommandBuffer",
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

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::SubmitCommandBuffer",

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

bool ExposurePass::UpdateMipCount ( VkDevice device,
    uint32_t mipCount,
    ExposureSpecialization const &specInfo
) noexcept
{
    if ( mipCount == _mipCount )
        return true;

    _program.Destroy ( device );

    if ( !_program.Init ( device, &specInfo ) ) [[unlikely]]
        return false;

    _mipCount = mipCount;
    return true;
}

bool ExposurePass::UpdateSyncMip5 ( android_vulkan::Renderer &renderer,
    VkDevice device,
    ResourceHeap &resourceHeap,
    VkExtent2D const &resolution
) noexcept
{
    if ( resolution.width == _mip5resolution.width && resolution.height == _mip5resolution.height ) [[unlikely]]
        return true;

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

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::UpdateSyncMip5",

        "Can't create image"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _syncMip5, VK_OBJECT_TYPE_IMAGE, "Exposure mip #5" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _syncMip5, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _syncMip5Memory._memory,
            _syncMip5Memory._offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,

            // FUCK - remove namespace
            "Can't allocate image memory (pbr::windows::ExposurePass::UpdateSyncMip5)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindImageMemory ( device, _syncMip5, _syncMip5Memory._memory, _syncMip5Memory._offset ),

            // FUCK - remove namespace
            "pbr::windows::ExposurePass::UpdateSyncMip5",

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

        // FUCK - remove namespace
        "pbr::windows::ExposurePass::UpdateSyncMip5",

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
    _mip5resolution = resolution;
    return true;
}

float ExposurePass::ExposureValueToLuma ( float exposureValue ) noexcept
{
    // https://knarkowicz.wordpress.com/2016/01/09/automatic-exposure/
    return 0.125F * std::exp2 ( exposureValue );
}

} // namespace pbr::windows
