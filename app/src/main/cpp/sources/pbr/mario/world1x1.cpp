#include <pbr/mario/world1x1.h>
#include <pbr/cube_map_manager.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <gamepad.h>
#include <global_force_gravity.h>


namespace pbr::mario {

constexpr static char const SCENE[] = "pbr/assets/world-1-1.scene";

constexpr static GXVec3 FREE_FALL_ACCELERATION ( 0.0F, -9.81F, 0.0F );

constexpr static uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr static uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

//----------------------------------------------------------------------------------------------------------------------

bool World1x1::IsReady () noexcept
{
    return _isReady;
}

bool World1x1::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    if ( !_scene.ExecuteInputEvents () )
        return false;

    auto const dt = static_cast<float> ( deltaTime );

    if ( !_scene.OnPrePhysics ( deltaTime ) )
        return false;

    _physics.Simulate ( dt );

    if ( !_scene.OnPostPhysics ( deltaTime ) )
        return false;

    _renderSession.Begin ( _scene.GetActiveCameraLocalMatrix (), _scene.GetActiveCameraProjectionMatrix () );
    _scene.Submit ( renderer, _renderSession, _fontStorage );

    if ( !_scene.OnUpdate ( deltaTime ) )
        return false;

    return _renderSession.End ( renderer, deltaTime );
}

bool World1x1::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_fontStorage.Init () )
        return false;

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "pbr::mario::World1x1::OnInit",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::mario::World1x1::_commandPool" )

    if ( !_renderSession.OnInitDevice ( renderer ) || !_scene.OnInitDevice ( renderer, _physics ) )
        return false;

    if ( !_scene.LoadScene ( renderer, SCENE, _commandPool ) )
        return false;

    _isReady = true;
    _renderSession.FreeTransferResources ( renderer );

    return CreatePhysics ();
}

void World1x1::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _scene.OnDestroyDevice ();
    _renderSession.OnDestroyDevice ( renderer );

    DestroyCommandPool ( renderer.GetDevice () );
    _physics.Reset ();

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
    CubeMapManager::Destroy ( renderer );

    _fontStorage.Destroy ();
    _isReady = false;
}

bool World1x1::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D resolution = renderer.GetViewportResolution ();
    resolution.width = resolution.width * RESOLUTION_SCALE_WIDTH / 100U;
    resolution.height = resolution.height * RESOLUTION_SCALE_HEIGHT / 100U;

    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution ) )
        return false;

    bool const result = _scene.OnResolutionChanged ( resolution,
        static_cast<double> ( surfaceResolution.width ) / static_cast<double> ( surfaceResolution.height )
    );

    if ( !result )
        return false;

    _physics.Resume ();
    _scene.OnResume ();

    return true;
}

void World1x1::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    _scene.OnPause ();
    _physics.Pause ();
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
}

bool World1x1::CreatePhysics () noexcept
{
    _physics.SetTimeSpeed ( 1.0F );

    if ( _physics.AddGlobalForce ( std::make_shared<android_vulkan::GlobalForceGravity> ( FREE_FALL_ACCELERATION ) ) )
        return true;

    android_vulkan::LogError ( "pbr::mario::World1x1::CreatePhysics - Can't add gravity." );
    return false;
}

void World1x1::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "pbr::mario::World1x1::_commandPool" )
}

} // namespace pbr::mario
