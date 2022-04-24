#include <pbr/mario/world1x1.h>
#include <pbr/mario/brick.h>
#include <pbr/mario/obstacle.h>
#include <pbr/mario/pipe_x1.h>
#include <pbr/mario/pipe_x2.h>
#include <pbr/mario/riddle.h>
#include <pbr/component.h>
#include <pbr/cube_map_manager.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/scene_desc.h>
#include <pbr/script_component.h>
#include <pbr/script_engine.h>
#include <gamepad.h>
#include <global_force_gravity.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr::mario {

constexpr static char const SCENE[] = "pbr/assets/world-1-1.scene";

constexpr static GXVec3 FREE_FALL_ACCELERATION ( 0.0F, -9.81F, 0.0F );

constexpr static uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr static uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

[[maybe_unused]] constexpr static uint32_t SCENE_DESC_FORMAT_VERSION = 2U;

//----------------------------------------------------------------------------------------------------------------------

bool World1x1::IsReady () noexcept
{
    return _isReady;
}

bool World1x1::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );

    if ( !_scene.OnPrePhysics ( deltaTime ) )
        return false;

    _mario.OnUpdate ();
    _physics.Simulate ( dt );

    if ( !_scene.OnPostPhysics ( deltaTime ) )
        return false;

    _camera.OnUpdate ( dt );

    _renderSession.Begin ( _camera.GetLocalMatrix (), _camera.GetProjectionMatrix () );
    _scene.Submit ( _renderSession );

    if ( !_scene.OnUpdate ( deltaTime ) )
        return false;

    return _renderSession.End ( renderer, deltaTime );
}

bool World1x1::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
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

    if ( !_renderSession.OnInitDevice ( renderer, _commandPool ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    if ( !_scene.OnInitDevice ( _physics ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    ComponentRef script = std::make_shared<ScriptComponent> ( "av://assets/Scripts/player.lua",
        "{ msg = 'hello world', state = 1 }"
    );

    ActorRef logic = std::make_shared<Actor> ( "Logic" );
    logic->AppendComponent ( script );
    _scene.AppendActor ( logic );

    if ( !UploadGPUContent ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    _renderSession.FreeTransferResources ( device, _commandPool );

    if ( !CreatePhysics () )
    {
        OnDestroyDevice ( device );
        return false;
    }

    return true;
}

void World1x1::OnDestroyDevice ( VkDevice device ) noexcept
{
    _scene.OnDestroyDevice ();
    _mario.Destroy ();
    _renderSession.OnDestroyDevice ( device );
    DestroyCommandPool ( device );
    _physics.Reset ();

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
    CubeMapManager::Destroy ( device );

    _isReady = false;
}

bool World1x1::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D resolution = renderer.GetViewportResolution ();
    resolution.width = resolution.width * RESOLUTION_SCALE_WIDTH / 100U;
    resolution.height = resolution.height * RESOLUTION_SCALE_HEIGHT / 100U;

    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();
    _camera.OnResolutionChanged ( surfaceResolution );

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution, _commandPool ) )
        return false;

    _physics.Resume ();
    _mario.CaptureInput ();
    return true;
}

void World1x1::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    Mario::ReleaseInput ();
    _physics.Pause ();
    _renderSession.OnSwapchainDestroyed ( device );
}

bool World1x1::CreatePhysics () noexcept
{
    _physics.SetTimeSpeed ( 1.0F );

    if ( !_physics.AddGlobalForce ( std::make_shared<android_vulkan::GlobalForceGravity> ( FREE_FALL_ACCELERATION ) ) )
    {
        android_vulkan::LogError ( "pbr::mario::World1x1::CreatePhysics - Can't add gravity." );
        return false;
    }

    constexpr float boundaryFriction = 0.0F;
    constexpr float normalFriction = 0.7F;

    constexpr Obstacle::Item const boundary[] =
    {
        {
            ._location = GXVec3 ( 4.0F, 7.2F, 84.8F ),
            ._size = GXVec3 ( 8.0F, 16.0F, 169.6F ),
            ._friction = boundaryFriction
        },

        {
            ._location = GXVec3 ( -5.6F, 7.2F, 84.8F ),
            ._size = GXVec3 ( 8.0F, 16.0F, 169.6F ),
            ._friction = boundaryFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 7.2F, -4.0F ),
            ._size = GXVec3 ( 17.6F, 16.0F, 8.0F ),
            ._friction = boundaryFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 7.2F, 173.6F ),
            ._size = GXVec3 ( 17.6F, 16.0F, 8.0F ),
            ._friction = boundaryFriction
        }
    };

    Obstacle::Spawn ( boundary, _scene, "Boundary" );

    constexpr Obstacle::Item const concrete1[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 2.4F, 109.2F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 2.4F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.0F, 107.6F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = 0.8F
        },

        {
            ._location = GXVec3 ( -0.8F, 3.6F, 109.6F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 1.6F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 4.4F, 110.0F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( concrete1, _scene, "Concrete #1" );

    constexpr Obstacle::Item const concrete2[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 2.4F, 113.2F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 2.4F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.0F, 114.8F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 3.6F, 112.8F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 1.6F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 4.4F, 112.4F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.4F, 125.2F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 2.4F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.0F, 126.8F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 3.6F, 124.8F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 1.6F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 4.4F, 124.4F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( concrete2, _scene, "Concrete #2" );

    constexpr Obstacle::Item const concrete3[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 2.8F, 121.2F ),
            ._size = GXVec3 ( 1.6F, 2.4F, 2.4F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.0F, 119.2F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 1.6F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.8F, 119.6F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 4.4F, 121.6F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 1.6F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( concrete3, _scene, "Concrete #3" );

    constexpr Obstacle::Item const concrete4[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 3.6F, 150.0F ),
            ._size = GXVec3 ( 1.6F, 4.0F, 4.0F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.4F, 146.8F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 2.4F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 2.0F, 145.2F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 3.6F, 147.2F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 1.6F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 4.4F, 147.6F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 6.4F, 150.8F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 2.4F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 6.0F, 149.2F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        },

        {
            ._location = GXVec3 ( -0.8F, 7.6F, 151.2F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 1.6F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( concrete4, _scene, "Concrete #4" );

    constexpr Obstacle::Item const concrete5[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 2.0F, 158.8F ),
            ._size = GXVec3 ( 1.6F, 0.8F, 0.8F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( concrete5, _scene, "Concrete #5" );

    constexpr Obstacle::Item const ground1[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 0.8F, 62.8F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 12.0F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( ground1, _scene, "Ground #1" );

    constexpr Obstacle::Item const ground2[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 0.8F, 146.8F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 45.6F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( ground2, _scene, "Ground #2" );

    constexpr Obstacle::Item const ground3[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 0.8F, 96.8F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 51.2F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( ground3, _scene, "Ground #3" );

    constexpr Obstacle::Item const ground4[] =
    {
        {
            ._location = GXVec3 ( -0.8F, 0.8F, 27.6F ),
            ._size = GXVec3 ( 1.6F, 1.6F, 55.2F ),
            ._friction = normalFriction
        }
    };

    Obstacle::Spawn ( ground4, _scene, "Ground #4" );
    return true;
}

void World1x1::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "pbr::mario::World1x1::_commandPool" )
}

bool World1x1::UploadGPUContent ( android_vulkan::Renderer &renderer ) noexcept
{
    android_vulkan::File file ( SCENE );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* sceneDesc = reinterpret_cast<pbr::SceneDesc const*> ( data );

    // Sanity checks.
    static_assert ( sizeof ( GXVec3 ) == sizeof ( sceneDesc->_viewerLocation ) );
    assert ( sceneDesc->_formatVersion == SCENE_DESC_FORMAT_VERSION );

    auto const comBuffs = static_cast<size_t> (
        sceneDesc->_textureCount +
        sceneDesc->_meshCount +
        sceneDesc->_envMapCount +
        PipeBase::CommandBufferCountRequirement () +
        Brick::CommandBufferCountRequirement () +
        Riddle::CommandBufferCountRequirement () +
        Mario::CommandBufferCountRequirement ()
    );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( comBuffs )
    };

    _commandBuffers.resize ( comBuffs );
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, _commandBuffers.data () ),
        "pbr::mario::World1x1::UploadGPUContent",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    VkCommandBuffer const* commandBuffers = _commandBuffers.data ();
    uint8_t const* readPointer = data + sizeof ( pbr::SceneDesc );

    auto const limit = static_cast<size_t> ( sceneDesc->_componentCount );
    size_t consumed = 0U;
    size_t read = 0U;

    for ( size_t i = 0U; i < limit; ++i )
    {
        ComponentRef component = Component::Create ( renderer,
            consumed,
            read,
            *reinterpret_cast<ComponentDesc const*> ( readPointer ),
            data,
            commandBuffers
        );

        if ( component )
        {
            ActorRef actor = std::make_shared<Actor> ();
            actor->AppendComponent ( component );
            _scene.AppendActor ( actor );
        }

        commandBuffers += consumed;
        readPointer += read;
    }

    PipeX1::Spawn ( renderer, commandBuffers, _scene, 0.0F, 27.2F, 716.8F );
    PipeX1::Spawn ( renderer, commandBuffers, _scene, 0.0F, 52.8F, 972.8F );
    PipeX1::Spawn ( renderer, commandBuffers, _scene, 0.0F, 27.2F, 4172.8F );
    PipeX1::Spawn ( renderer, commandBuffers, _scene, 0.0F, 27.2F, 4582.4F );
    PipeX2::Spawn ( renderer, commandBuffers, _scene, 0.0F, 51.2F, 1177.6F );
    PipeX2::Spawn ( renderer, commandBuffers, _scene, 0.0F, 51.2F, 1459.2F );

    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 512.0F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 563.2F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 614.4F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 1971.2F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 2022.4F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2048.0F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2073.6F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2099.2F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2124.8F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2150.4F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2176.0F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2201.6F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2227.2F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2329.6F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2355.2F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2380.8F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 2406.4F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 2560.0F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 3020.8F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 3097.6F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 3123.2F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 3148.8F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 3276.8F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 3353.6F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 3302.4F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 3328.0F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 4300.8F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 4326.4F );
    Brick::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 4377.6F );

    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 409.6F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 537.6F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 588.8F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 563.2F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 1996.8F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 2713.6F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 2790.4F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 2867.2F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2406.4F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 2790.4F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 3302.4F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 230.4F, 3328.0F );
    Riddle::Spawn ( renderer, commandBuffers, _scene, -12.8F, 128.0F, 4352.0F );

    _mario.Init ( renderer, commandBuffers, _scene, -0.8F, 4.4F, 3.00189F );
    _camera.SetTarget ( _mario );
    _camera.Focus ();

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "pbr::mario::World1x1::UploadGPUContent",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    _mario.FreeTransferResources ( device );
    _scene.FreeTransferResources ( device );
    _isReady = true;
    return true;
}

} // namespace pbr::mario
