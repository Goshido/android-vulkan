#include <precompiled_headers.hpp>
#include <platform/windows/pbr/present_pass.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::windows {

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

void PresentPass::OnDestroyDevice ( VkDevice device ) noexcept
{
    for ( auto renderEnd : _renderEnd )
        vkDestroySemaphore ( device, renderEnd, nullptr );

    _renderEnd.clear ();
    _renderEnd.shrink_to_fit ();
}

bool PresentPass::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    size_t const imageCount = renderer.GetPresentImageCount ();
    size_t const semaphoreCount = _renderEnd.size ();

    if ( imageCount <= semaphoreCount ) [[likely]]
        return true;

    constexpr VkSemaphoreCreateInfo semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkDevice device = renderer.GetDevice ();
    _renderEnd.resize ( imageCount );
    VkSemaphore* s = _renderEnd.data () + semaphoreCount;

    for ( size_t i = semaphoreCount; i < imageCount; ++i )
    {
        VkSemaphore &semaphore = s[ i ];

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &semaphore ),
            // FUCK - remove namespace
            "pbr::windows::PresentPass::OnSwapchainCreated",
            "Can't create render target acquired semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, semaphore, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain image #%zu", i )
    }

    return true;
}

void PresentPass::Begin ( android_vulkan::Renderer const &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    _colorAttachment.imageView = renderer.GetPresentImageView ( static_cast<size_t> ( _swapchainImageIndex ) );
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

    bool result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        // FUCK - remove namespace
        "pbr::windows::PresentPass::Execute",
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
            // FUCK - remove namespace
            "pbr::windows::PresentPass::End",
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

} // namespace pbr::windows
