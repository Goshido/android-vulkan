#include <pbr/present_pass.h>


namespace pbr {

bool PresentPass::AcquirePresentTarget ( android_vulkan::Renderer &renderer ) noexcept
{
    _framebufferIndex = UINT32_MAX;

    return android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            UINT64_MAX,
            _targetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &_framebufferIndex
        ),

        "pbr::PresentPass::AcquirePresentTarget",
        "Can't get presentation image index"
    );
}

bool PresentPass::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !CreateRenderPass ( renderer ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !CreateFramebuffers ( renderer ) )
    {
        Destroy ( device );
        return false;
    }

    VkExtent2D const& resolution = renderer.GetSurfaceSize ();

    if ( !_program.Init ( renderer, _renderPass, 0U, resolution ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkSemaphoreCreateInfo const semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderEndSemaphore ),
        "pbr::PresentPass::Init",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "pbr::PresentPass::_renderEndSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_targetAcquiredSemaphore ),
        "pbr::PresentPass::Init",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "pbr::PresentPass::_targetAcquiredSemaphore" )
    InitCommonStructures ( resolution );
    return true;
}

void PresentPass::Destroy ( VkDevice device ) noexcept
{
    if ( _targetAcquiredSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _targetAcquiredSemaphore, nullptr );
        _targetAcquiredSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "pbr::PresentPass::_targetAcquiredSemaphore" )
    }

    if ( _renderEndSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _renderEndSemaphore, nullptr );
        _renderEndSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "pbr::PresentPass::_renderEndSemaphore" )
    }

    _program.Destroy ( device );
    DestroyFramebuffers ( device );

    if ( _renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _renderPass, nullptr );
        _renderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "pbr::PresentPass::_renderPass" )
    }
}

bool PresentPass::Execute ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkDescriptorSet presentTarget,
    VkFence fence
) noexcept
{
    _renderInfo.framebuffer = _framebuffers[ _framebufferIndex ];

    vkCmdBeginRenderPass ( commandBuffer, &_renderInfo, VK_SUBPASS_CONTENTS_INLINE );
    _program.Bind ( commandBuffer );
    _program.SetData ( commandBuffer, presentTarget, renderer.GetPresentationEngineTransform () );

    vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
    vkCmdEndRenderPass ( commandBuffer );

    bool result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "pbr::PresentPass::Execute",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    _submitInfo.pCommandBuffers = &commandBuffer;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfo, fence ),
        "pbr::PresentPass::End",
        "Can't submit geometry render command buffer"
    );

    if ( !result )
        return false;

    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    _presentInfo.pResults = &presentResult;
    _presentInfo.pSwapchains = &renderer.GetSwapchain ();
    _presentInfo.pImageIndices = &_framebufferIndex;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueuePresentKHR ( renderer.GetQueue (), &_presentInfo ),
        "pbr::PresentPass::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult,
        "pbr::PresentPass::EndFrame",
        "Present queue has been failed"
    );
}

bool PresentPass::CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( framebufferCount );
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    VkExtent2D const& resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 1U;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.layers = 1U;

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "pbr::PresentPass::CreateFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result )
            return false;

        _framebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "pbr::PresentPass::_framebuffers" )
    }

    return true;
}

void PresentPass::DestroyFramebuffers ( VkDevice device ) noexcept
{
    if ( _framebuffers.empty () )
        return;

    for ( auto framebuffer : _framebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::PresentPass::_framebuffers" )
    }

    _framebuffers.clear ();
}

bool PresentPass::CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept
{
    VkAttachmentDescription const attachment[] =
    {
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
        }
    };

    constexpr static VkAttachmentReference const references[] =
    {
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkSubpassDescription const subpasses[] =
    {
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0U,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( references ) ),
            .pColorAttachments = references,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        }
    };

    VkRenderPassCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachment ) ),
        .pAttachments = attachment,
        .subpassCount = static_cast<uint32_t> ( std::size ( subpasses ) ),
        .pSubpasses = subpasses,
        .dependencyCount = 0U,
        .pDependencies = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &info, nullptr, &_renderPass ),
        "pbr::PresentPass::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "pbr::PresentPass::_renderPass" )
    return true;
}

void PresentPass::InitCommonStructures ( VkExtent2D const &resolution ) noexcept
{
    _renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _renderInfo.pNext = nullptr;
    _renderInfo.renderPass = _renderPass;

    _renderInfo.renderArea =
    {
        .offset
        {
            .x = 0,
            .y = 0
        },

        .extent = resolution
    };

    _renderInfo.clearValueCount = 0U;
    _renderInfo.pClearValues = nullptr;

    _presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    _presentInfo.pNext = nullptr;
    _presentInfo.waitSemaphoreCount = 1U;
    _presentInfo.pWaitSemaphores = &_renderEndSemaphore;
    _presentInfo.swapchainCount = 1U;

    constexpr static VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    _submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfo.pNext = nullptr;
    _submitInfo.waitSemaphoreCount = 1U;
    _submitInfo.pWaitSemaphores = &_targetAcquiredSemaphore;
    _submitInfo.pWaitDstStageMask = &waitStage;
    _submitInfo.commandBufferCount = 1U;
    _submitInfo.signalSemaphoreCount = 1U;
    _submitInfo.pSignalSemaphores = &_renderEndSemaphore;
}

} // namespace pbr
