#include <pbr/pbr_game.h>
#include <vulkan_utils.h>

namespace pbr {

PBRGame::PBRGame ():
    _commandPool ( VK_NULL_HANDLE ),
    _frameBuffers {},
    _gBuffer {},
    _lightupRenderPass ( VK_NULL_HANDLE )
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
    _gBuffer.Destroy ( renderer );

    DestroyFramebuffers ( renderer );
    DestroyRenderPasses ( renderer );
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
    const size_t framebufferCount = renderer.GetPresentImageCount ();
    _frameBuffers.reserve ( framebufferCount );

    const VkExtent2D& resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.attachmentCount = 1U;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.layers = 1U;
    framebufferInfo.renderPass = _lightupRenderPass;

    VkDevice device = renderer.GetDevice ();
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );

        const bool result = renderer.CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "PBRGame::CreateFramebuffers",
            "Can't create framebuffer"
        );

        if ( !result )
            return false;

        _frameBuffers.emplace_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "PBRGame::_framebuffers" )
    }

    return true;
}

void PBRGame::DestroyFramebuffers ( android_vulkan::Renderer &renderer )
{
    if ( _frameBuffers.empty() )
        return;

    VkDevice device = renderer.GetDevice ();

    for ( const auto framebuffer : _frameBuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "PBRGame::_framebuffers" )
    }

    _frameBuffers.clear ();
}

bool PBRGame::CreateRenderPasses ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"PBRGame::CreateRenderPasses - Implement me!");
    return false;
}

void PBRGame::DestroyRenderPasses ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"PBRGame::DestroyRenderPasses - Implement me!");
}

} // namespace pbr
