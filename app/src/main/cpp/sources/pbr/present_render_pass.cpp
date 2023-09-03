#include <pbr/present_render_pass.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

bool PresentRenderPass::AcquirePresentTarget ( android_vulkan::Renderer &renderer,
    size_t &swapchainImageIndex
) noexcept
{
    AV_TRACE ( "Acquire swapchain image" )

    _framebufferIndex = std::numeric_limits<uint32_t>::max ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            std::numeric_limits<uint64_t>::max (),
            _targetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &_framebufferIndex
        ),

        "pbr::PresentRenderPass::AcquirePresentTarget",
        "Can't get presentation image index"
    );

    swapchainImageIndex = _framebufferIndex;
    return result;
}

VkRenderPass PresentRenderPass::GetRenderPass () const noexcept
{
    return _renderInfo.renderPass;
}

bool PresentRenderPass::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    constexpr VkSemaphoreCreateInfo const semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderEndSemaphore ),
        "pbr::PresentRenderPass::OnInitDevice",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "pbr::PresentRenderPass::_renderEndSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_targetAcquiredSemaphore ),
        "pbr::PresentRenderPass::OnInitDevice",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "pbr::PresentRenderPass::_targetAcquiredSemaphore" )

    InitCommonStructures ();
    return true;
}

void PresentRenderPass::OnDestroyDevice ( VkDevice device ) noexcept
{
    if ( _targetAcquiredSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _targetAcquiredSemaphore, nullptr );
        _targetAcquiredSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "pbr::PresentRenderPass::_targetAcquiredSemaphore" )
    }

    if ( _renderEndSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _renderEndSemaphore, nullptr );
        _renderEndSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "pbr::PresentRenderPass::_renderEndSemaphore" )
    }

    DestroyFramebuffers ( device );

    _renderInfo.renderArea.extent =
    {
        .width = 0U,
        .height = 0U
    };

    if ( _renderInfo.renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderInfo.renderPass, nullptr );
    _renderInfo.renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "pbr::PresentRenderPass::_renderPass" )
}

bool PresentRenderPass::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    VkExtent2D const &resolution = renderer.GetSurfaceSize ();
    bool const hasRenderPass = _renderInfo.renderPass != VK_NULL_HANDLE;

    if ( hasRenderPass && !CreateFramebuffers ( renderer, device, resolution ) )
        return false;

    if ( !CreateRenderPass ( renderer, device, resolution ) )
        return false;

    return !_framebuffers.empty () || CreateFramebuffers ( renderer, device, resolution );
}

void PresentRenderPass::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    DestroyFramebuffers ( device );
}

void PresentRenderPass::Begin ( VkCommandBuffer commandBuffer ) noexcept
{
    _renderInfo.framebuffer = _framebuffers[ _framebufferIndex ];
    vkCmdBeginRenderPass ( commandBuffer, &_renderInfo, VK_SUBPASS_CONTENTS_INLINE );
}

bool PresentRenderPass::End ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    vkCmdEndRenderPass ( commandBuffer );

    bool result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "pbr::PresentRenderPass::Execute",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    _submitInfo.pCommandBuffers = &commandBuffer;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfo, fence ),
        "pbr::PresentRenderPass::End",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    _presentInfo.pResults = &presentResult;
    _presentInfo.pSwapchains = &renderer.GetSwapchain ();
    _presentInfo.pImageIndices = &_framebufferIndex;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueuePresentKHR ( renderer.GetQueue (), &_presentInfo ),
        "pbr::PresentRenderPass::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult,
        "pbr::PresentRenderPass::EndFrame",
        "Present queue has been failed"
    );
}

bool PresentRenderPass::CreateFramebuffers ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkExtent2D const &resolution
) noexcept
{
    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( framebufferCount );

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

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );
        VkFramebuffer framebuffer;

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "pbr::PresentRenderPass::CreateFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result )
            return false;

        _framebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "pbr::PresentRenderPass::_framebuffers" )
    }

    return true;
}

void PresentRenderPass::DestroyFramebuffers ( VkDevice device ) noexcept
{
    if ( _framebuffers.empty () )
        return;

    for ( auto framebuffer : _framebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::PresentRenderPass::_framebuffers" )
    }

    _framebuffers.clear ();
}

bool PresentRenderPass::CreateRenderPass ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkExtent2D const &resolution
) noexcept
{
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
        "pbr::PresentRenderPass::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "pbr::PresentRenderPass::_renderPass" )
    _renderInfo.renderArea.extent = resolution;
    return true;
}

void PresentRenderPass::InitCommonStructures () noexcept
{
    _renderInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = VK_NULL_HANDLE,
        .framebuffer = VK_NULL_HANDLE,

        .renderArea =
        {
            .offset
            {
                .x = 0,
                .y = 0
            },

            .extent =
            {
                .width = 0U,
                .height = 0U
            }
        },

        .clearValueCount = 0U,
        .pClearValues = nullptr
    };

    _presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_renderEndSemaphore,
        .swapchainCount = 1U,
        .pSwapchains = nullptr,
        .pImageIndices = nullptr,
        .pResults = nullptr
    };

    constexpr static VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    _submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_targetAcquiredSemaphore,
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1U,
        .pCommandBuffers = nullptr,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &_renderEndSemaphore
    };
}

} // namespace pbr
