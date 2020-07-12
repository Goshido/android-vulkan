#include <pbr/pbr_game.h>
#include <vulkan_utils.h>

namespace pbr {

PBRGame::PBRGame ():
    _presentFrameBuffers {},
    _presentRenderPass ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool PBRGame::IsReady ()
{
    //assert ( !"PBRGame::IsReady - Implement me!" );
    return false;
}

bool PBRGame::OnInit ( android_vulkan::Renderer &renderer )
{
    if ( !CreatePresentRenderPass ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreatePresentFramebuffer ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !_renderSession.Init ( renderer, _presentRenderPass ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    assert ( !"PBRGame::OnInit - Implement me!" );
    return true;
}

bool PBRGame::OnFrame ( android_vulkan::Renderer& /*renderer*/, double /*deltaTime*/ )
{
    assert ( !"PBRGame::OnFrame - Implement me!" );
    return false;
}

bool PBRGame::OnDestroy ( android_vulkan::Renderer &renderer )
{
    _renderSession.Destroy( renderer );

    DestroyPresentFramebuffer ( renderer );

    if ( _presentRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( renderer.GetDevice (), _presentRenderPass, nullptr );
        _presentRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "PBRGame::_presentRenderPass" )
    }

    assert ( !"PBRGame::OnDestroy - Implement me!" );
    return true;
}

bool PBRGame::CreatePresentFramebuffer ( android_vulkan::Renderer &renderer )
{
    const size_t framebufferCount = renderer.GetPresentImageCount ();
    _presentFrameBuffers.reserve ( framebufferCount );

    const VkExtent2D& resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.attachmentCount = 1U;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.layers = 1U;
    framebufferInfo.renderPass = _presentRenderPass;

    VkDevice device = renderer.GetDevice ();
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );

        const bool result = renderer.CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "PBRGame::CreatePresentFramebuffer",
            "Can't create present framebuffer"
        );

        if ( !result )
            return false;

        _presentFrameBuffers.emplace_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "PBRGame::_presentFrameBuffers" )
    }

    return true;
}

void PBRGame::DestroyPresentFramebuffer ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    if ( _presentFrameBuffers.empty() )
        return;

    for ( const auto framebuffer : _presentFrameBuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "PBRGame::_presentFrameBuffers" )
    }

    _presentFrameBuffers.clear ();
}

bool PBRGame::CreatePresentRenderPass ( android_vulkan::Renderer &renderer )
{
    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0U;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0U;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1U;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.inputAttachmentCount = 0U;
    subpass.pInputAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.pResolveAttachments = nullptr;
    subpass.preserveAttachmentCount = 0U;
    subpass.pPreserveAttachments = nullptr;

    VkAttachmentDescription attachments;
    attachments.flags = 0U;
    attachments.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments.format = renderer.GetSurfaceFormat ();
    attachments.samples = VK_SAMPLE_COUNT_1_BIT;
    attachments.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0U;
    renderPassInfo.subpassCount = 1U;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0U;
    renderPassInfo.pDependencies = nullptr;
    renderPassInfo.attachmentCount = 1U;
    renderPassInfo.pAttachments = &attachments;

    const bool result = renderer.CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_presentRenderPass ),
        "PBRGame::CreatePresentRenderPass",
        "Can't create present render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "PBRGame::_presentRenderPass" )
    return true;
}

} // namespace pbr
