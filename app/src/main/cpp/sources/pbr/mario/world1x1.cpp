#include <pbr/mario/world1x1.h>
#include <pbr/mario/brick.h>
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
#include <shape_box.h>

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

    ScriptEngine& scriptEngine = ScriptEngine::GetInstance ();

    if ( !scriptEngine.Execute ( deltaTime ) )
        return false;

    _mario.OnUpdate ();
    _physics.Simulate ( dt );
    _camera.OnUpdate ( dt );
    _renderSession.Begin ( _camera.GetLocalMatrix (), _camera.GetProjectionMatrix () );

    _mario.Submit ( _renderSession );
    _scene.Submit ( _renderSession );

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

    if ( !_scene.OnInitDevice () )
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
    _renderSession.OnDestroyDevice ( device );
    DestroyCommandPool ( device );

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

    auto append = [ & ] ( float x,
        float y,
        float z,
        float w,
        float h,
        float d,
        float friction,
        char const* name
    ) noexcept -> bool {
        android_vulkan::RigidBodyRef body = std::make_shared<android_vulkan::RigidBody> ();
        body->EnableKinematic ();
        body->SetLocation ( x, y, z, false );

        android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( w, h, d );
        shape->SetFriction ( friction );
        body->SetShape ( shape, false );

        if ( _physics.AddRigidBody ( body ) )
            return true;

        android_vulkan::LogError ( "pbr::mario::World1x1::CreatePhysics::append - Can't append %s", name );
        return false;
    };

    constexpr float boundaryFriction = 0.0F;
    constexpr float normalFriction = 0.7F;

    if ( !append ( 4.0F, 7.2F, 84.8F, 8.0F, 16.0F, 169.6F, boundaryFriction, "boundary-001" ) )
        return false;

    if ( !append ( -5.6F, 7.2F, 84.8F, 8.0F, 16.0F, 169.6F, boundaryFriction, "boundary-002" ) )
        return false;

    if ( !append ( -0.8F, 7.2F, -4.0F, 17.6F, 16.0F, 8.0F, boundaryFriction, "boundary-003" ) )
        return false;

    if ( !append ( -0.8F, 7.2F, 173.6F, 17.6F, 16.0F, 8.0F, boundaryFriction, "boundary-004" ) )
        return false;

    if ( !append ( -0.8F, 2.4F, 109.2F, 1.6F, 1.6F, 2.4F, normalFriction, "concrete-001-collider-001-005" ) )
        return false;

    if ( !append ( -0.8F, 2.0F, 107.6F, 1.6F, 0.8F, 0.8F, 0.8F, "concrete-001-collider-001-006" ) )
        return false;

    if ( !append ( -0.8F, 3.6F, 109.6F, 1.6F, 0.8F, 1.6F, normalFriction, "concrete-001-collider-001-007" ) )
        return false;

    if ( !append ( -0.8F, 4.4F, 110.0F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-001-collider-001-008" ) )
        return false;

    if ( !append ( -0.8F, 2.4F, 113.2F, 1.6F, 1.6F, 2.4F, normalFriction, "concrete-002-collider-001-005" ) )
        return false;

    if ( !append ( -0.8F, 2.0F, 114.8F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-002-collider-001-006" ) )
        return false;

    if ( !append ( -0.8F, 3.6F, 112.8F, 1.6F, 0.8F, 1.6F, normalFriction, "concrete-002-collider-001-007" ) )
        return false;

    if ( !append ( -0.8F, 4.4F, 112.4F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-002-collider-001-008" ) )
        return false;

    if ( !append ( -0.8F, 2.4F, 125.2F, 1.6F, 1.6F, 2.4F, normalFriction, "concrete-002-collider-001-009" ) )
        return false;

    if ( !append ( -0.8F, 2.0F, 126.8F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-002-collider-001-010" ) )
        return false;

    if ( !append ( -0.8F, 3.6F, 124.8F, 1.6F, 0.8F, 1.6F, normalFriction, "concrete-002-collider-001-011" ) )
        return false;

    if ( !append ( -0.8F, 4.4F, 124.4F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-002-collider-001-012" ) )
        return false;

    if ( !append ( -0.8F, 2.8F, 121.2F, 1.6F, 2.4F, 2.4F, normalFriction, "concrete-003-collider-001-005" ) )
        return false;

    if ( !append ( -0.8F, 2.0F, 119.2F, 1.6F, 0.8F, 1.6F, normalFriction, "concrete-003-collider-001-006" ) )
        return false;

    if ( !append ( -0.8F, 2.8F, 119.6F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-003-collider-001-007" ) )
        return false;

    if ( !append ( -0.8F, 4.4F, 121.6F, 1.6F, 0.8F, 1.6F, normalFriction, "concrete-003-collider-001-008" ) )
        return false;

    if ( !append ( -0.8F, 3.6F, 150.0F, 1.6F, 4.0F, 4.0F, normalFriction, "concrete-004-collider-001-009" ) )
        return false;

    if ( !append ( -0.8F, 2.4F, 146.8F, 1.6F, 1.6F, 2.4F, normalFriction, "concrete-004-collider-001-010" ) )
        return false;

    if ( !append ( -0.8F, 2.0F, 145.2F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-004-collider-001-011" ) )
        return false;

    if ( !append ( -0.8F, 3.6F, 147.2F, 1.6F, 0.8F, 1.6F, normalFriction, "concrete-004-collider-001-012" ) )
        return false;

    if ( !append ( -0.8F, 4.4F, 147.6F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-004-collider-001-013" ) )
        return false;

    if ( !append ( -0.8F, 6.4F, 150.8F, 1.6F, 1.6F, 2.4F, normalFriction, "concrete-004-collider-001-014" ) )
        return false;

    if ( !append ( -0.8F, 6.0F, 149.2F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-004-collider-001-015" ) )
        return false;

    if ( !append ( -0.8F, 7.6F, 151.2F, 1.6F, 0.8F, 1.6F, normalFriction, "concrete-004-collider-001-016" ) )
        return false;

    if ( !append ( -0.8F, 2.0F, 158.8F, 1.6F, 0.8F, 0.8F, normalFriction, "concrete-005-collider-002" ) )
        return false;

    if ( !append ( -0.8F, 0.8F, 62.8F, 1.6F, 1.6F, 12.0F, normalFriction, "ground-x15-collider-002" ) )
        return false;

    if ( !append ( -0.8F, 0.8F, 146.8F, 1.6F, 1.6F, 45.6F, normalFriction, "ground-x57-collider-002" ) )
        return false;

    if ( !append ( -0.8F, 0.8F, 96.8F, 1.6F, 1.6F, 51.2F, normalFriction, "ground-x64-collider-002" ) )
        return false;

    if ( !append ( -0.8F, 0.8F, 27.6F, 1.6F, 1.6F, 55.2F, normalFriction, "ground-x69-collider-002" ) )
        return false;

    if ( _physics.AddRigidBody ( _mario.GetRigidBody () ) )
        return true;

    android_vulkan::LogError ( "pbr::mario::World1x1::CreatePhysics - Can't append mario." );
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

    // Note it's generic lambda. Kinda templated lambda.
    auto appendActor = [ & ] ( auto* actor, float x, float y, float z ) noexcept {
        actor->Init ( renderer, consumed, commandBuffers, _scene, _physics, x, y, z );
        commandBuffers += consumed;
    };

    appendActor ( new PipeX1 (), 0.0F, 27.2F, 716.8F );
    appendActor ( new PipeX1 (), 0.0F, 52.8F, 972.8F );
    appendActor ( new PipeX1 (), 0.0F, 27.2F, 4172.8F );
    appendActor ( new PipeX1 (), 0.0F, 27.2F, 4582.4F );
    appendActor ( new PipeX2 (), 0.0F, 51.2F, 1177.6F );
    appendActor ( new PipeX2 (), 0.0F, 51.2F, 1459.2F );

    appendActor ( new Brick (), -12.8F, 128.0F, 512.0F );
    appendActor ( new Brick (), -12.8F, 128.0F, 563.2F );
    appendActor ( new Brick (), -12.8F, 128.0F, 614.4F );
    appendActor ( new Brick (), -12.8F, 128.0F, 1971.2F );
    appendActor ( new Brick (), -12.8F, 128.0F, 2022.4F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2048.0F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2073.6F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2099.2F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2124.8F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2150.4F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2176.0F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2201.6F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2227.2F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2329.6F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2355.2F );
    appendActor ( new Brick (), -12.8F, 230.4F, 2380.8F );
    appendActor ( new Brick (), -12.8F, 128.0F, 2406.4F );
    appendActor ( new Brick (), -12.8F, 128.0F, 2560.0F );
    appendActor ( new Brick (), -12.8F, 128.0F, 3020.8F );
    appendActor ( new Brick (), -12.8F, 230.4F, 3097.6F );
    appendActor ( new Brick (), -12.8F, 230.4F, 3123.2F );
    appendActor ( new Brick (), -12.8F, 230.4F, 3148.8F );
    appendActor ( new Brick (), -12.8F, 230.4F, 3276.8F );
    appendActor ( new Brick (), -12.8F, 230.4F, 3353.6F );
    appendActor ( new Brick (), -12.8F, 128.0F, 3302.4F );
    appendActor ( new Brick (), -12.8F, 128.0F, 3328.0F );
    appendActor ( new Brick (), -12.8F, 128.0F, 4300.8F );
    appendActor ( new Brick (), -12.8F, 128.0F, 4326.4F );
    appendActor ( new Brick (), -12.8F, 128.0F, 4377.6F );

    appendActor ( new Riddle (), -12.8F, 128.0F, 409.6F );
    appendActor ( new Riddle (), -12.8F, 128.0F, 537.6F );
    appendActor ( new Riddle (), -12.8F, 128.0F, 588.8F );
    appendActor ( new Riddle (), -12.8F, 230.4F, 563.2F );
    appendActor ( new Riddle (), -12.8F, 128.0F, 1996.8F );
    appendActor ( new Riddle (), -12.8F, 128.0F, 2713.6F );
    appendActor ( new Riddle (), -12.8F, 128.0F, 2790.4F );
    appendActor ( new Riddle (), -12.8F, 128.0F, 2867.2F );
    appendActor ( new Riddle (), -12.8F, 230.4F, 2406.4F );
    appendActor ( new Riddle (), -12.8F, 230.4F, 2790.4F );
    appendActor ( new Riddle (), -12.8F, 230.4F, 3302.4F );
    appendActor ( new Riddle (), -12.8F, 230.4F, 3328.0F );
    appendActor ( new Riddle (), -12.8F, 128.0F, 4352.0F );

    _mario.Init ( renderer, consumed, commandBuffers, -0.8F, 4.4F, 3.00189F );
    commandBuffers += consumed;

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
