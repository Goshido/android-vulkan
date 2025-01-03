#include <precompiled_headers.hpp>
#include <gamepad.hpp>
#include <global_force_gravity.hpp>
#include <logger.hpp>
#include <pbr/cube_map_manager.hpp>
#include <pbr/material_manager.hpp>
#include <pbr/mesh_manager.hpp>
#include <pbr/universal_game.hpp>


namespace pbr {

namespace {

constexpr GXVec3 FREE_FALL_ACCELERATION ( 0.0F, -9.81F, 0.0F );

constexpr uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UniversalGame::UniversalGame ( std::string &&sceneFile ) noexcept:
    _sceneFile ( std::move ( sceneFile ) )
{
    // NOTHING
}

bool UniversalGame::IsReady () noexcept
{
    return _isReady;
}

bool UniversalGame::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    if ( !_scene.ExecuteInputEvents () ) [[unlikely]]
        return false;

    auto const dt = static_cast<float> ( deltaTime );

    if ( !_scene.OnPrePhysics ( dt ) ) [[unlikely]]
        return false;

    _physics.Simulate ( dt );

    if ( !_scene.OnPostPhysics ( dt ) ) [[unlikely]]
        return false;

    _scene.OnUpdateAnimations ( dt, _renderSession.GetWritingCommandBufferIndex () );

    if ( !_scene.OnAnimationUpdated ( dt ) || !_scene.OnUpdate ( dt ) ) [[unlikely]]
        return false;

    _renderSession.Begin ( _scene.GetActiveCameraLocalMatrix (), _scene.GetActiveCameraProjectionMatrix () );
    _scene.Submit ( renderer );
    return _renderSession.End ( renderer, deltaTime );
}

bool UniversalGame::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
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
        "pbr::UniversalGame::OnInit",
        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _commandPool, VK_OBJECT_TYPE_COMMAND_POOL, "Scene loader" )

    result = _renderSession.OnInitDevice ( renderer ) &&
        _scene.OnInitDevice ( renderer, _renderSession, _physics ) &&
        _scene.LoadScene ( renderer, _sceneFile.c_str (), _commandPool );

    if ( !result ) [[unlikely]]
        return false;

    _isReady = true;
    _renderSession.FreeTransferResources ( renderer );
    return CreatePhysics ();
}

void UniversalGame::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _scene.OnDestroyDevice ();
    _renderSession.OnDestroyDevice ( renderer );

    DestroyCommandPool ( renderer.GetDevice () );
    _physics.Reset ();

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
    CubeMapManager::Destroy ( renderer );

    _isReady = false;
}

bool UniversalGame::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D resolution = renderer.GetViewportResolution ();

    resolution.width = resolution.width * RESOLUTION_SCALE_WIDTH / 100U;
    resolution.height = resolution.height * RESOLUTION_SCALE_HEIGHT / 100U;

    VkExtent2D const &surfaceResolution = renderer.GetViewportResolution ();

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution ) ) [[unlikely]]
        return false;

    bool const result = _scene.OnResolutionChanged ( resolution,
        static_cast<float> ( surfaceResolution.width ) / static_cast<float> ( surfaceResolution.height )
    );

    if ( !result ) [[unlikely]]
        return false;

    _physics.Resume ();
    _scene.OnResume ();

    return true;
}

void UniversalGame::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    _scene.OnPause ();
    _physics.Pause ();
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
}

bool UniversalGame::OnInitSoundSystem () noexcept
{
    return _scene.OnInitSoundSystem ();
}

void UniversalGame::OnDestroySoundSystem () noexcept
{
    _scene.OnDestroySoundSystem ();
}

bool UniversalGame::CreatePhysics () noexcept
{
    _physics.SetTimeSpeed ( 1.0F );

    if ( _physics.AddGlobalForce ( std::make_shared<android_vulkan::GlobalForceGravity> ( FREE_FALL_ACCELERATION ) ) )
    {
        [[likely]]
        return true;
    }

    android_vulkan::LogError ( "pbr::UniversalGame::CreatePhysics - Can't add gravity." );
    return false;
}

void UniversalGame::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
}

} // namespace pbr
