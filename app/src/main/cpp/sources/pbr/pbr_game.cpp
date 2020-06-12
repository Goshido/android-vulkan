#include <pbr/pbr_game.h>

namespace pbr {

PBRGame::PBRGame ():
    _commandPool ( VK_NULL_HANDLE ),
    _depthImage ( VK_NULL_HANDLE ),
    _depthMemory ( VK_NULL_HANDLE ),
    _depthView ( VK_NULL_HANDLE ),
    _frameBuffers {}
{
    // NOTHING
}

bool PBRGame::IsReady ()
{
    assert ( !"PBRGame::IsReady - Implement me!" );
    return false;
}

bool PBRGame::OnInit ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"PBRGame::OnInit - Implement me!" );
    return false;
}

bool PBRGame::OnFrame ( android_vulkan::Renderer& /*renderer*/, double /*deltaTime*/ )
{
    assert ( !"PBRGame::OnFrame - Implement me!" );
    return false;
}

bool PBRGame::OnDestroy ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"PBRGame::OnDestroy - Implement me!" );
    return false;
}

} // namespace pbr
