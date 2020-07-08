#include <pbr/pbr_game.h>
#include <vulkan_utils.h>

namespace pbr {

constexpr static size_t GBUFFER_ATTACHMENT_COUNT = 5U;

PBRGame::PBRGame ():
    _commandPool ( VK_NULL_HANDLE ),
    _gBuffer {},
    _gBufferFramebuffer ( VK_NULL_HANDLE ),
    _gBufferRenderPass ( VK_NULL_HANDLE ),
    _opaqueProgram {},
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
    if ( !CreateCommandPool ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !_gBuffer.Init ( renderer.GetViewportResolution (), renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateRenderPasses ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateFramebuffers ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !_opaqueProgram.Init ( renderer, _gBufferRenderPass, _gBuffer.GetResolution () ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    assert ( !"PBRGame::OnInit - Implement me!" );
    return false;
}

bool PBRGame::OnFrame ( android_vulkan::Renderer& /*renderer*/, double /*deltaTime*/ )
{
    assert ( !"PBRGame::OnFrame - Implement me!" );
    return false;
}

bool PBRGame::OnDestroy ( android_vulkan::Renderer &renderer )
{
    _opaqueProgram.Destroy( renderer );

    DestroyFramebuffers ( renderer );
    DestroyRenderPasses ( renderer );

    _gBuffer.Destroy ( renderer );

    DestroyCommandPool ( renderer );

    assert ( !"PBRGame::OnDestroy - Implement me!" );
    return false;
}

bool PBRGame::CreateCommandPool ( android_vulkan::Renderer &renderer )
{
    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = renderer.GetQueueFamilyIndex ();

    const bool result = renderer.CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &poolInfo, nullptr, &_commandPool ),
        "PBRGame::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )
    return true;
}

void PBRGame::DestroyCommandPool ( android_vulkan::Renderer &renderer )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )
}

bool PBRGame::CreateFramebuffers ( android_vulkan::Renderer &renderer )
{
    if ( !CreateGBufferFramebuffer ( renderer ) )
        return false;

    return CreatePresentFramebuffer ( renderer );
}

bool PBRGame::CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer )
{
    const VkExtent2D& resolution = _gBuffer.GetResolution ();

    VkImageView attachments[ GBUFFER_ATTACHMENT_COUNT ] =
    {
        _gBuffer.GetAlbedo ().GetImageView (),
        _gBuffer.GetEmission ().GetImageView (),
        _gBuffer.GetNormal ().GetImageView (),
        _gBuffer.GetParams ().GetImageView (),
        _gBuffer.GetDepthStencil ().GetImageView ()
    };

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.attachmentCount = std::size ( attachments );
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.layers = 1U;
    framebufferInfo.renderPass = _gBufferRenderPass;

    const bool result = renderer.CheckVkResult (
        vkCreateFramebuffer ( renderer.GetDevice (), &framebufferInfo, nullptr, &_gBufferFramebuffer ),
        "PBRGame::CreateGBufferFramebuffer",
        "Can't create GBuffer framebuffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_FRAMEBUFFER ( "PBRGame::_gBufferFramebuffer" )
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

void PBRGame::DestroyFramebuffers ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    if ( _gBufferFramebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _gBufferFramebuffer, nullptr );
        _gBufferFramebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "PBRGame::_gBufferFramebuffer" )
    }

    if ( _presentFrameBuffers.empty() )
        return;

    for ( const auto framebuffer : _presentFrameBuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "PBRGame::_presentFrameBuffers" )
    }

    _presentFrameBuffers.clear ();
}

bool PBRGame::CreateGBufferRenderPass ( android_vulkan::Renderer &renderer )
{
    constexpr const size_t colorAttachmentCount = GBUFFER_ATTACHMENT_COUNT - 1U;

    VkAttachmentReference colorAttachmentReferences[ colorAttachmentCount ];
    VkAttachmentReference& albedoRef = colorAttachmentReferences[ 0U ];
    albedoRef.attachment = 0U;
    albedoRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference& emissionRef = colorAttachmentReferences[ 1U ];
    emissionRef.attachment = 1U;
    emissionRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference& normalRef = colorAttachmentReferences[ 2U ];
    normalRef.attachment = 2U;
    normalRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference& paramsRef = colorAttachmentReferences[ 3U ];
    paramsRef.attachment = 3U;
    paramsRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 4U;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0U;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t> ( colorAttachmentCount );
    subpass.pColorAttachments = colorAttachmentReferences;
    subpass.inputAttachmentCount = 0U;
    subpass.pInputAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
    subpass.pResolveAttachments = nullptr;
    subpass.preserveAttachmentCount = 0U;
    subpass.pPreserveAttachments = nullptr;

    VkAttachmentDescription attachments[ GBUFFER_ATTACHMENT_COUNT ];
    VkAttachmentDescription& albedoAttachment = attachments[ 0U ];
    albedoAttachment.flags = 0U;
    albedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    albedoAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    albedoAttachment.format = _gBuffer.GetAlbedo ().GetFormat ();
    albedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    albedoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    albedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& emissionAttachment = attachments[ 1U ];
    emissionAttachment.flags = 0U;
    emissionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    emissionAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    emissionAttachment.format = _gBuffer.GetEmission ().GetFormat ();
    emissionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    emissionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    emissionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    emissionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    emissionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& normalAttachment = attachments[ 2U ];
    normalAttachment.flags = 0U;
    normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    normalAttachment.format = _gBuffer.GetNormal ().GetFormat ();
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& paramsAttachment = attachments[ 3U ];
    paramsAttachment.flags = 0U;
    paramsAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    paramsAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    paramsAttachment.format = _gBuffer.GetParams ().GetFormat ();
    paramsAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    paramsAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    paramsAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    paramsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    paramsAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& depthAttachment = attachments[ 4U ];
    depthAttachment.flags = 0U;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.format = _gBuffer.GetDepthStencil ().GetFormat ();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0U;
    renderPassInfo.subpassCount = 1U;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0U;
    renderPassInfo.pDependencies = nullptr;
    renderPassInfo.attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) );
    renderPassInfo.pAttachments = attachments;

    const bool result = renderer.CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_gBufferRenderPass ),
        "PBRGame::CreateGBufferRenderPass",
        "Can't create GBuffer render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "PBRGame::_gBufferRenderPass" )
    return true;
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

bool PBRGame::CreateRenderPasses ( android_vulkan::Renderer &renderer )
{
    if ( !CreateGBufferRenderPass ( renderer ) )
        return false;

    return CreatePresentRenderPass ( renderer );
}

void PBRGame::DestroyRenderPasses ( android_vulkan::Renderer &renderer )
{
    if ( _presentRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( renderer.GetDevice (), _presentRenderPass, nullptr );
        _presentRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "PBRGame::_presentRenderPass" )
    }

    if ( _gBufferRenderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( renderer.GetDevice (), _gBufferRenderPass, nullptr );
    _gBufferRenderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "PBRGame::_gBufferRenderPass" )
}

} // namespace pbr
