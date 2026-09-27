#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <platform/windows/pbr/present_pass.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

VkResult PresentPass::AcquirePresentTarget ( android_vulkan::Renderer &renderer, VkSemaphore acquire ) noexcept
{
    AV_TRACE ( "Acquire swapchain image" )

    return vkAcquireNextImageKHR ( renderer.GetDevice (),
        renderer.GetSwapchain (),
        std::numeric_limits<uint64_t>::max (),
        acquire,
        VK_NULL_HANDLE,
        &_swapchainImageIndex
    );
}

void PresentPass::OnDestroyDevice ( VkDevice device, ResourceHeap &resourceHeap ) noexcept
{
    FreeHeapResources ( resourceHeap );

    for ( auto renderEnd : _renderEnd )
        vkDestroySemaphore ( device, renderEnd, nullptr );

    _renderEnd.clear ();
    _renderEnd.shrink_to_fit ();

    _heapIndex.clear ();
    _heapIndex.shrink_to_fit ();
}

bool PresentPass::OnSwapchainCreated ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept
{
    FreeHeapResources ( resourceHeap );
    size_t const imageCount = renderer.GetPresentImageCount ();
    _heapIndex.resize ( imageCount );
    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < imageCount; ++i )
    {
        if ( auto idx = resourceHeap.RegisterStorageImage ( device, renderer.GetPresentImageView ( i ) ); idx )
        {
            [[likely]]
            _heapIndex[ i ] = std::move ( idx );
            continue;
        }

        AV_ASSERT ( false )
        return false;
    }

    size_t const semaphoreCount = _renderEnd.size ();
    _renderingInfo.renderArea.extent = renderer.GetSurfaceSize ();

    if ( imageCount <= semaphoreCount ) [[likely]]
        return true;

    constexpr VkSemaphoreCreateInfo semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    _renderEnd.resize ( imageCount );
    VkSemaphore* s = _renderEnd.data () + semaphoreCount;

    for ( size_t i = semaphoreCount; i < imageCount; ++i )
    {
        VkSemaphore &semaphore = s[ i ];

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &semaphore ),
            "pbr::PresentPass::OnSwapchainCreated",
            "Can't create render target acquired semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, semaphore, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain image #%zu", i )
    }

    return true;
}

SwapchainInfo PresentPass::GetSwapchainInfo ( android_vulkan::Renderer const &renderer ) const noexcept
{
    auto const idx = static_cast<size_t> ( _swapchainImageIndex );

    return
    {
        ._image = renderer.GetPresentImage ( static_cast<size_t> ( idx ) ),
        ._view = renderer.GetPresentImageView ( static_cast<size_t> ( idx ) ),
        ._idx = *_heapIndex[ idx ]
    };
}

void PresentPass::Begin ( android_vulkan::Renderer const &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    auto const idx = static_cast<size_t> ( _swapchainImageIndex );
    _barrierStart.image = renderer.GetPresentImage ( idx );
    _barrierEnd.image = _barrierStart.image;
    _depInfo.pImageMemoryBarriers = &_barrierStart;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    _colorAttachment.imageView = renderer.GetPresentImageView ( idx );
    vkCmdBeginRendering ( commandBuffer, &_renderingInfo );
}

void PresentPass::Pause ( VkCommandBuffer commandBuffer ) noexcept
{
    vkCmdEndRendering ( commandBuffer );
}

void PresentPass::Continue ( VkCommandBuffer commandBuffer, VkImage swapchainImage, VkImageView swapchainView ) noexcept
{
    _barrierContiue.image = swapchainImage;
    _depInfo.pImageMemoryBarriers = &_barrierContiue;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    _colorAttachment.imageView = swapchainView;
    vkCmdBeginRendering ( commandBuffer, &_renderingInfo );
}

std::optional<VkResult> PresentPass::End ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkSemaphore acquire,
    VkFence fence,
    std::mutex* submitMutex
) noexcept
{
    vkCmdEndRendering ( commandBuffer );

    _depInfo.pImageMemoryBarriers = &_barrierEnd;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    bool result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "pbr::PresentPass::Execute",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
        return std::nullopt;

    VkSemaphore* renderEnd = &_renderEnd[ _swapchainImageIndex ];
    VkQueue queue = renderer.GetQueue ();

    _submitInfo.pWaitSemaphores = &acquire;
    _submitInfo.pCommandBuffers = &commandBuffer;
    _submitInfo.pSignalSemaphores = renderEnd;

    auto const submit = [ & ]() noexcept -> bool
    {
        return android_vulkan::Renderer::CheckVkResult ( vkQueueSubmit ( queue, 1U, &_submitInfo, fence ),
            "pbr::PresentPass::End",
            "Can't submit command buffer"
        );
    };

    if ( !submitMutex )
    {
        result = submit ();
    }
    else
    {
        std::lock_guard const lock ( *submitMutex );
        result = submit ();
    }

    if ( !result ) [[unlikely]]
        return std::nullopt;

    _presentInfo.pSwapchains = &renderer.GetSwapchain ();
    _presentInfo.pImageIndices = &_swapchainImageIndex;
    _presentInfo.pWaitSemaphores = renderEnd;
    return std::optional<VkResult> { vkQueuePresentKHR ( queue, &_presentInfo ) };
}

void PresentPass::FreeHeapResources ( ResourceHeap& resourceHeap ) noexcept
{
    for ( auto const &idx : _heapIndex )
    {
        if ( idx ) [[likely]]
        {
            resourceHeap.UnregisterResource ( *idx );
        }
    }
}

} // namespace pbr
