#include <precompiled_headers.hpp>
#include <platform/android/pbr/present_pass.hpp>
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
        &_framebufferIndex
    );
}

VkRenderPass PresentPass::GetRenderPass () const noexcept
{
    return _renderInfo.renderPass;
}

void PresentPass::OnDestroyDevice ( VkDevice device ) noexcept
{
    DestroyFramebuffers ( device );

    _renderInfo.renderArea.extent =
    {
        .width = 0U,
        .height = 0U
    };

    if ( VkRenderPass &renderPass = _renderInfo.renderPass; renderPass != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyRenderPass ( device, std::exchange ( renderPass, VK_NULL_HANDLE ), nullptr );
    }
}

bool PresentPass::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    VkExtent2D const &resolution = renderer.GetSurfaceSize ();

    return !( ( _renderInfo.renderPass != VK_NULL_HANDLE ) && !CreateFramebuffers ( renderer, device, resolution ) ) &&
        CreateRenderPass ( renderer, device, resolution ) &&
        ( !_framebufferInfo.empty () || CreateFramebuffers ( renderer, device, resolution ) );
}

void PresentPass::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    DestroyFramebuffers ( device );
}

void PresentPass::Begin ( VkCommandBuffer commandBuffer ) noexcept
{
    _renderInfo.framebuffer = _framebufferInfo[ _framebufferIndex ]._framebuffer;
    vkCmdBeginRenderPass ( commandBuffer, &_renderInfo, VK_SUBPASS_CONTENTS_INLINE );
}

std::optional<VkResult> PresentPass::End ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkSemaphore acquire,
    VkFence fence,
    std::mutex* submitMutex
) noexcept
{
    vkCmdEndRenderPass ( commandBuffer );

    bool result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "pbr::PresentPass::Execute",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
        return std::nullopt;

    VkSemaphore* renderEnd = &_framebufferInfo[ _framebufferIndex ]._renderEnd;
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
    _presentInfo.pImageIndices = &_framebufferIndex;
    _presentInfo.pWaitSemaphores = renderEnd;
    return std::optional<VkResult> { vkQueuePresentKHR ( queue, &_presentInfo ) };
}

bool PresentPass::CreateFramebuffers ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkExtent2D const &resolution
) noexcept
{
    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _framebufferInfo.resize ( framebufferCount );

    VkFramebufferCreateInfo framebufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _renderInfo.renderPass,
        .attachmentCount = 1U,
        .pAttachments = nullptr,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    constexpr VkSemaphoreCreateInfo semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        FramebufferInfo &info = _framebufferInfo[ i ];

        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &info._framebuffer ),
            "pbr::PresentPass::CreateFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "Swapchain image #%zu", i )

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &info._renderEnd ),
            "pbr::PresentPass::CreateFramebuffers",
            "Can't create render target acquired semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._renderEnd, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain image #%zu", i )
    }

    return true;
}

void PresentPass::DestroyFramebuffers ( VkDevice device ) noexcept
{
    if ( _framebufferInfo.empty () )
        return;

    for ( auto &fbInfo : _framebufferInfo )
    {
        if ( fbInfo._framebuffer != VK_NULL_HANDLE ) [[likely]]
            vkDestroyFramebuffer ( device, fbInfo._framebuffer, nullptr );

        if ( fbInfo._renderEnd != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroySemaphore ( device, fbInfo._renderEnd, nullptr );
        }
    }

    _framebufferInfo.clear ();
    _framebufferInfo.shrink_to_fit ();
}

bool PresentPass::CreateRenderPass ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkExtent2D const &resolution
) noexcept
{
     _renderInfo.renderArea.extent = resolution;

    if ( _renderInfo.renderPass != VK_NULL_HANDLE )
        return true;

    VkAttachmentDescription const attachment
    {
        .flags = 0U,
        .format = renderer.GetSurfaceFormat (),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    constexpr static VkAttachmentReference reference
    {
        .attachment = 0U,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    constexpr VkSubpassDescription subpass
    {
        .flags = 0U,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0U,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1U,
        .pColorAttachments = &reference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0U,
        .pPreserveAttachments = nullptr
    };

    constexpr VkSubpassDependency dependency
    {
        .srcSubpass = 0U,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_NONE,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    };

    VkRenderPassCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = 1U,
        .pAttachments = &attachment,
        .subpassCount = 1U,
        .pSubpasses = &subpass,
        .dependencyCount = 1U,
        .pDependencies = &dependency
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &info, nullptr, &_renderInfo.renderPass ),
        "pbr::PresentPass::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderInfo.renderPass, VK_OBJECT_TYPE_RENDER_PASS, "Present" )
    return true;
}

} // namespace pbr
