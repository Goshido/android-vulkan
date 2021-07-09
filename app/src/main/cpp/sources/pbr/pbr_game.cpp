#include <pbr/pbr_game.h>
#include <gamepad.h>
#include <global_force_gravity.h>
#include <shape_box.h>
#include <vulkan_utils.h>
#include <pbr/component.h>
#include <pbr/cube_map_manager.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/scene_desc.h>
#include <pbr/static_mesh_component.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static char const* SCENES[] =
{
    "pbr/assets/N7_ADM_Reception.scene",
    "pbr/assets/N7_ENG_Injection.scene",
    "pbr/assets/physics-sandbox.scene"
};

constexpr static size_t const ACTIVE_SCENE = 0U;
static_assert ( std::size ( SCENES ) > ACTIVE_SCENE );

[[maybe_unused]] constexpr static uint32_t const SCENE_DESC_FORMAT_VERSION = 2U;

constexpr static float const FIELD_OF_VIEW = 75.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+4F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 80U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 70U;

constexpr static GXVec3 const FREE_FALL_ACCELERATION ( 0.0F, -9.81F, 0.0F );

//----------------------------------------------------------------------------------------------------------------------

PBRGame::PBRGame () noexcept:
    _camera {},
    _commandPool ( VK_NULL_HANDLE ),
    _commandBuffers {},
    _cube {},
    _cubeBody {},
    _floor {},
    _floorBody {},
    _floorPhase ( 0.0F ),
    _colorA {},
    _colorB {},
    _defaultColor {},
    _physics {},
    _sphereMesh {},
    _sphereMaterial {},
    _renderSession {},
    _components {}
{
    // NOTHING
}

bool PBRGame::IsReady ()
{
    return !_components.empty ();
}

bool PBRGame::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime )
{
    auto const dt = static_cast<float> ( deltaTime );

    _physics.Simulate ( dt );
    UpdatePhysicsActors ( dt );

    _camera.Update ( dt );
    GXMat4 const& cameraLocal = _camera.GetLocalMatrix ();
    _renderSession.Begin ( cameraLocal, _camera.GetProjectionMatrix () );

    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLightComponent*> ( _cameraLight.get () );

    GXVec3 lightLocation {};
    cameraLocal.GetW ( lightLocation );
    light.SetLocation ( lightLocation );

    GXMat4 transform {};
    constexpr float const rendererScale = 32.0F;
    constexpr float const sphereSize = 0.02F * rendererScale;
    constexpr GXVec3 const sphereDims ( sphereSize * 0.5F, sphereSize * 0.5F, sphereSize * 0.5F );
    transform.Scale ( sphereSize, sphereSize, sphereSize );

    auto submit = [ & ] ( GXVec3 const &loc, android_vulkan::Half4 const &color ) noexcept {
        GXVec3 location {};
        location.Multiply ( loc, rendererScale );
        transform.SetW ( location );

        GXAABB bounds{};
        GXVec3 v {};
        v.Sum ( location, sphereDims );
        bounds.AddVertex ( v );

        v.Subtract ( location, sphereDims );
        bounds.AddVertex ( v );

        _renderSession.SubmitMesh ( _sphereMesh,
            _sphereMaterial,
            transform,
            bounds,
            color,
            _defaultColor,
            _defaultColor,
            _defaultColor
        );
    };

    for ( auto const& manifold : _physics.GetContactManifolds () )
    {
        android_vulkan::Contact* contact =  manifold._contacts;

        for ( size_t i = 0U; i < manifold._contactCount; ++i )
        {
            android_vulkan::Contact const& c = contact[ i ];
            submit ( c._pointAfterResolve, _colorB );
        }
    }

    for ( auto& component : _components )
        component->Submit ( _renderSession );

    return _renderSession.End ( renderer, deltaTime );
}

bool PBRGame::OnInitDevice ( android_vulkan::Renderer &renderer )
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
        "PBRGame::OnInit",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )

   if ( !_renderSession.OnInitDevice ( renderer ) )
   {
       OnDestroyDevice ( device );
       return false;
   }

    if ( !CreateSceneManual ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

//    if ( !UploadGPUContent( renderer ) )
//    {
//        OnDestroyDevice ( device );
//        return false;
//    }

    return true;
}

void PBRGame::OnDestroyDevice ( VkDevice device )
{
    DestroyPhysics ();
    _floor = nullptr;
    _cube = nullptr;
    _components.clear ();
    _renderSession.OnDestroyDevice ( device );
    DestroyCommandPool ( device );

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
    CubeMapManager::Destroy ( device );
}

bool PBRGame::OnSwapchainCreated ( android_vulkan::Renderer &renderer )
{
    VkExtent2D resolution = renderer.GetViewportResolution ();
    resolution.width = resolution.width * RESOLUTION_SCALE_WIDTH / 100U;
    resolution.height = resolution.height * RESOLUTION_SCALE_HEIGHT / 100U;

    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        surfaceResolution.width / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution, _commandPool ) )
        return false;

    _camera.CaptureInput ();
    _physics.Resume ();

    return true;
}

void PBRGame::OnSwapchainDestroyed ( VkDevice device )
{
    _physics.Pause ();
    Camera::ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( device );
}

void PBRGame::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )
}

[[maybe_unused]] bool PBRGame::UploadGPUContent ( android_vulkan::Renderer& renderer ) noexcept
{
    android_vulkan::File file ( SCENES[ ACTIVE_SCENE ] );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* sceneDesc = reinterpret_cast<pbr::SceneDesc const*> ( data );

    // Sanity checks.
    static_assert ( sizeof ( GXVec3 ) == sizeof ( sceneDesc->_viewerLocation ) );
    assert ( sceneDesc->_formatVersion == SCENE_DESC_FORMAT_VERSION );

    _camera.SetLocation ( *reinterpret_cast<GXVec3 const*> ( &sceneDesc->_viewerLocation ) );
    _camera.SetRotation ( sceneDesc->_viewerPitch, sceneDesc->_viewerYaw );
    _camera.Update ( 0.0F );

    auto const comBuffs = static_cast<size_t> (
        sceneDesc->_textureCount +
        sceneDesc->_meshCount +
        sceneDesc->_envMapCount
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
        "PBRGame::UploadGPUContent",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    VkCommandBuffer const* commandBuffers = _commandBuffers.data ();
    uint8_t const* readPointer = data + sizeof ( pbr::SceneDesc );

    auto const limit = static_cast<size_t const> ( sceneDesc->_componentCount );
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
            _components.push_back ( component );

        commandBuffers += consumed;
        readPointer += read;
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "PBRGame::UploadGPUContent",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto& component : _components )
        component->FreeTransferResources ( device );

    return true;
}

bool PBRGame::CreateSceneManual ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.SetLocation ( GXVec3 ( 0.0F, 0.0F, -14.0F ) );
    _camera.Update ( 0.0F );

    _physics.SetTimeSpeed ( 1.0F );

    if ( !_physics.AddGlobalForce ( std::make_shared<android_vulkan::GlobalForceGravity> ( FREE_FALL_ACCELERATION ) ) )
    {
        android_vulkan::LogError ( "PBR::PBRGame::CreateSceneManual - Can't add gravity." );
        return false;
    }

    constexpr size_t const CUBE_COMMAND_BUFFERS = 1U;
    constexpr size_t const DEFAULT_MATERIAL_COMMAND_BUFFERS = 5U;
    constexpr size_t const SPHERE_COMMAND_BUFFERS = 1U;
    constexpr size_t const UNLIT_MATERIAL_COMMAND_BUFFERS = 5U;

    auto const comBuffs = static_cast<size_t> (
        CUBE_COMMAND_BUFFERS +
        DEFAULT_MATERIAL_COMMAND_BUFFERS +
        SPHERE_COMMAND_BUFFERS +
        UNLIT_MATERIAL_COMMAND_BUFFERS
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
        "PBRGame::CreateSceneManual",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    VkCommandBuffer* commandBuffers = _commandBuffers.data ();
    size_t consumed = 0U;

    _cube = std::make_shared<StaticMeshComponent> ( renderer,
        consumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/Props/PBR/Unlit.mtl",
        commandBuffers
    );

    commandBuffers += consumed;

    _colorA = android_vulkan::Half4 ( 115.0F / 255.0F, 185.0F / 255.0F, 0.0F, 1.0F );
    _colorB = android_vulkan::Half4 ( 42.0F / 255.0F, 127.0F / 230.0F, 203.0F, 1.0F );
    _defaultColor = android_vulkan::Half4 ( 1.0F, 1.0F, 1.0F, 1.0F );

    // NOLINTNEXTLINE
    auto& cubeComp = *static_cast<StaticMeshComponent*> ( _cube.get () );
    cubeComp.SetColor0 ( _colorA );
    _components.push_back ( _cube );

    _cubeBody = std::make_shared<android_vulkan::RigidBody> ();
    _cubeBody->SetMass ( 0.777F );
    _cubeBody->DisableKinematic ();
    _cubeBody->DisableSleep ();

    constexpr GXVec3 const cubeSpawn ( 0.0F, 1.0F, 0.8F );
    _cubeBody->SetLocation ( cubeSpawn );

//    GXQuat rotation {};
//    rotation.FromAxisAngle ( 1.0F, 0.0F, 0.0F, GX_MATH_HALF_PI * 0.5F );
//    _cubeBody->SetRotation ( rotation );

//    GXVec3 const angularVelocity ( 0.0F, 20.0F, 0.0F );
//    _cubeBody->SetVelocityAngular ( angularVelocity );

    android_vulkan::ShapeRef cubeShape = std::make_shared<android_vulkan::ShapeBox> ( 0.9F, 0.5F, 0.4F );
    _cubeBody->SetShape ( cubeShape );

    GXVec3 point ( 0.4F, -0.1F, 0.05F );
    point.Sum ( point, cubeSpawn );

    GXVec3 const impulse ( -1.4106F, 2.0577F, 1.931F );
    _cubeBody->AddImpulse ( impulse, point );

    _floor = std::make_shared<StaticMeshComponent> ( renderer,
        consumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/Props/PBR/DefaultCSGEmissive.mtl",
        commandBuffers
    );

    commandBuffers += consumed;

    // NOLINTNEXTLINE
    auto& floorComp = *static_cast<StaticMeshComponent*> ( _floor.get () );
    floorComp.SetColor0 ( _defaultColor );
    _components.push_back ( _floor );

    _floorBody = std::make_shared<android_vulkan::RigidBody> ();
    _floorBody->EnableKinematic ();
    _floorBody->SetLocation ( 0.0F, -0.5F, 0.0F );

    android_vulkan::ShapeRef floorShape = std::make_shared<android_vulkan::ShapeBox> ( 32.0F, 0.5F, 32.0F );
    _floorBody->SetShape ( floorShape );

    if ( !_physics.AddRigidBody ( _floorBody ) )
    {
        android_vulkan::LogError ( "PBR::PBRGame::CreateSceneManual - Can't add rigid body (floor)." );
        return false;
    }

    if ( !_physics.AddRigidBody ( _cubeBody ) )
    {
        android_vulkan::LogError ( "PBR::PBRGame::CreateSceneManual - Can't add rigid body (cube)." );
        return false;
    }

    _sphereMesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        "pbr/system/unit-sphere.mesh2",
        *commandBuffers
    );

    commandBuffers += consumed;

    _sphereMaterial = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        consumed,
        "pbr/assets/System/Default.mtl",
        commandBuffers
    );

    _cameraLight = std::make_shared<PointLightComponent> ();

    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLightComponent*> ( _cameraLight.get () );
    light.SetIntensity ( 16.0F );
    light.SetBoundDimensions ( 1600.0F, 1600.0F, 1600.0F );
    _components.push_back ( _cameraLight );

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "PBRGame::CreateSceneManual",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto& component : _components )
        component->FreeTransferResources ( device );

    _sphereMesh->FreeTransferResources ( device );
    return true;
}

void PBRGame::DestroyPhysics () noexcept
{
    _floorBody = nullptr;
    _cubeBody = nullptr;
}

void PBRGame::UpdatePhysicsActors ( float deltaTime ) noexcept
{
    auto update = [] ( ComponentRef &component, android_vulkan::RigidBodyRef &body ) {
        // NOLINTNEXTLINE
        auto& c = *static_cast<StaticMeshComponent*> ( component.get () );

        // NOLINTNEXTLINE
        auto& s = static_cast<android_vulkan::ShapeBox&> ( body->GetShape () );

        constexpr float const physicsToRender = 32.0F;
        GXMat4 transform = body->GetTransform ();

        GXVec3 dims ( s.GetWidth (), s.GetHeight (), s.GetDepth () );
        dims.Multiply ( dims, physicsToRender );

        GXMat4 scale {};
        scale.Scale ( dims._data[ 0U ], dims._data[ 1U ], dims._data[ 2U ] );

        GXVec3 location {};
        transform.GetW ( location );
        location.Multiply ( location, physicsToRender );
        transform.SetW ( location );

        GXMat4 resultTransform {};
        resultTransform.Multiply ( scale, transform );

        c.SetTransform ( resultTransform );
    };

    update ( _cube, _cubeBody );
    update ( _floor, _floorBody );

    if ( _physics.IsPaused () )
        return;

    _floorPhase += deltaTime;
    _floorBody->SetVelocityLinear ( GXVec3 ( 1.6F * std::sin ( _floorPhase ), 0.0F, 0.0F ) );
}

} // namespace pbr
