#include <pbr/box_stack/box_stack.h>
#include <pbr/component.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/static_mesh_component.h>
#include <gamepad.h>
#include <global_force_gravity.h>
#include <shape_box.h>
#include <shape_sphere.h>


namespace pbr::box_stack {

constexpr static float const FIELD_OF_VIEW = 75.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+4F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 80U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 70U;

constexpr static GXVec3 const FREE_FALL_ACCELERATION ( 0.0F, -9.81F, 0.0F );
constexpr static float const TIME_SPEED = 1.0F;

constexpr static size_t const CUBES = 6U;

//----------------------------------------------------------------------------------------------------------------------

bool BoxStack::IsReady () noexcept
{
    return !_components.empty ();
}

bool BoxStack::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );

    _physics.Simulate ( dt );
    UpdatePhysicsActors ();

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

    auto submit = [ & ] ( GXVec3 const &loc, android_vulkan::Color32 const &color ) noexcept {
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
        android_vulkan::Contact* contact = manifold._contacts;

        for ( size_t i = 0U; i < manifold._contactCount; ++i )
        {
            android_vulkan::Contact const& c = contact[ i ];
            submit ( c._pointA, _colors[ 0U ] );
            submit ( c._pointB, _colors[ 2U ] );
        }
    }

    for ( auto& component : _components )
        component->Submit ( _renderSession );

    return _renderSession.End ( renderer, deltaTime );
}

bool BoxStack::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "BoxStack::OnInit",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "BoxStack::_commandPool" )

    if ( !_renderSession.OnInitDevice ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    InitColors ();

    if ( !CreateSceneManual ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    return true;
}

void BoxStack::OnDestroyDevice ( VkDevice device ) noexcept
{
    DestroyPhysics ();
    _cubes.clear ();
    _components.clear ();
    _renderSession.OnDestroyDevice ( device );
    DestroyCommandPool ( device );

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
}

bool BoxStack::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    VkExtent2D const resolution
    {
        .width = surfaceResolution.width * RESOLUTION_SCALE_WIDTH / 100U,
        .height = surfaceResolution.height * RESOLUTION_SCALE_HEIGHT / 100U
    };

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        static_cast<float> ( surfaceResolution.width ) / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution, _commandPool ) )
        return false;

    _camera.CaptureInput ();

    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();

    gamepad.BindKey ( this,
        &BoxStack::OnLeftBumper,
        android_vulkan::eGamepadKey::LeftBumper,
        android_vulkan::eButtonState::Down
    );

    gamepad.BindKey ( this,
        &BoxStack::OnRightBumper,
        android_vulkan::eGamepadKey::RightBumper,
        android_vulkan::eButtonState::Down
    );

    _physics.Resume ();
    return true;
}

void BoxStack::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::LeftBumper, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::RightBumper, android_vulkan::eButtonState::Down );

    _physics.Pause ();

    Camera::ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( device );
}

bool BoxStack::AppendCuboid ( android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers,
    size_t &commandBufferConsumed,
    std::string &&tag,
    ComponentRef &visual,
    char const* material,
    android_vulkan::Color32 const &color,
    android_vulkan::RigidBodyRef &physical,
    float x,
    float y,
    float z,
    float w,
    float h,
    float d
) noexcept
{
    visual = std::make_shared<StaticMeshComponent> ( renderer,
        commandBufferConsumed,
        "pbr/system/unit-cube.mesh2",
        material,
        commandBuffers
    );

    if ( !visual )
        return false;

    // NOLINTNEXTLINE
    auto& v = *static_cast<StaticMeshComponent*> ( visual.get () );
    v.SetColor0 ( color );

    _components.push_back ( visual );

    physical = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& ph = *physical.get ();
    ph.SetLocation ( x, y, z, true );
    ph.DisableKinematic ( true );
    ph.EnableSleep ();
    ph.SetTag ( std::move ( tag ) );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( w, h, d );
    ph.SetShape ( shape, true );

    if ( _physics.AddRigidBody ( physical ) )
    {
        UpdateCuboid ( visual, physical );
        return true;
    }

    android_vulkan::LogError ( "BoxStack::AppendCuboid - Can't add rigid body." );
    return false;
}

void BoxStack::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "BoxStack::_commandPool" )
}

bool BoxStack::CreateSceneManual ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.SetLocation ( GXVec3 ( 0.77F, 35.8F, -55.4F ) );
    _camera.Update ( 0.0F );

    _physics.SetTimeSpeed ( TIME_SPEED );

    if ( !_physics.AddGlobalForce ( std::make_shared<android_vulkan::GlobalForceGravity> ( FREE_FALL_ACCELERATION ) ) )
    {
        android_vulkan::LogError ( "BoxStack::CreateSceneManual - Can't add gravity." );
        return false;
    }

    constexpr size_t const CUBE_COMMAND_BUFFERS = 1U;
    constexpr size_t const DEFAULT_MATERIAL_COMMAND_BUFFERS = 5U;
    constexpr size_t const SPHERE_COMMAND_BUFFERS = 1U;
    constexpr size_t const UNLIT_MATERIAL_COMMAND_BUFFERS = 5U;

    constexpr size_t const comBuffs = CUBE_COMMAND_BUFFERS + DEFAULT_MATERIAL_COMMAND_BUFFERS + SPHERE_COMMAND_BUFFERS +
        UNLIT_MATERIAL_COMMAND_BUFFERS;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( comBuffs )
    };

    _commandBuffers.resize ( comBuffs );
    VkCommandBuffer* commandBuffers = _commandBuffers.data ();
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers ),
        "BoxStack::CreateSceneManual",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    size_t consumed = 0U;
    _defaultColor = android_vulkan::Color32 ( 255U, 255U, 255U, 255U );

    ComponentRef cuboid {};
    android_vulkan::RigidBodyRef body {};

    result = AppendCuboid ( renderer,
        commandBuffers,
        consumed,
        "Floor",
        cuboid,
        "pbr/assets/Props/PBR/DefaultCSGEmissive.mtl",
        _defaultColor,
        body,
        0.0F,
        -0.25F,
        0.0F,
        32.0F,
        0.5F,
        32.0F
    );

    if ( !result )
        return false;

    body->EnableKinematic ();

    commandBuffers += consumed;

    _cubes.resize ( CUBES );
    _cubeBodies.resize ( CUBES );
    size_t const paletteSize = _colors.size ();

    for ( size_t i = 0U; i < CUBES; ++i )
    {
        result = AppendCuboid ( renderer,
            commandBuffers,
            consumed,
            "Cube #" + std::to_string ( i ),
            _cubes[ i ],
            "pbr/assets/System/Default.mtl",
            _colors[ i % paletteSize ],
            _cubeBodies[ i ],
            0.0F,
            0.15F + 0.3F * static_cast<float> ( i ),
            0.0F,
            0.6F,
            0.3F,
            0.7F
        );

        if ( !result )
            return false;

        commandBuffers += consumed;

        android_vulkan::RigidBody& b = *_cubeBodies[ i ].get ();
        b.SetMass ( 7.77F, true );
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
        "BoxStack::CreateSceneManual",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto& component : _components )
        component->FreeTransferResources ( device );

    _sphereMesh->FreeTransferResources ( device );
    return true;
}

void BoxStack::DestroyPhysics () noexcept
{
    _cubeBodies.clear ();
}

void BoxStack::UpdatePhysicsActors () noexcept
{
    for ( size_t i = 0U; i < CUBES; ++i )
    {
        UpdateCuboid ( _cubes[ i ], _cubeBodies[ i ] );
    }
}

void BoxStack::InitColors () noexcept
{
    // NVIDIA green
    _colors[ 0U ] = android_vulkan::Color32 ( 115U, 185U, 0U, 255U );

    // Yellow
    _colors[ 1U ] = android_vulkan::Color32 ( 223U, 202U, 79U, 255U );

    // Red
    _colors[ 2U ] = android_vulkan::Color32 ( 223U, 79U, 88U, 255U );

    // Purple
    _colors[ 3U ] = android_vulkan::Color32 ( 223U, 79U, 210U, 255U );

    // Blue
    _colors[ 4U ] = android_vulkan::Color32 ( 100U, 79U, 223U, 255U );

    // Cyan
    _colors[ 5U ] = android_vulkan::Color32 ( 79U, 212U, 223U, 255U );

    // Green
    _colors[ 6U ] = android_vulkan::Color32 ( 79U, 223U, 107U, 255U );
}

void BoxStack::OnLeftBumper ( void* context ) noexcept
{
    auto& sandbox = *static_cast<BoxStack*> ( context );
    android_vulkan::Physics& p = sandbox._physics;

    if ( p.IsPaused () )
    {
        p.Resume ();
        return;
    }

    p.Pause ();
}

void BoxStack::OnRightBumper ( void* context ) noexcept
{
    auto& sandbox = *static_cast<BoxStack*> ( context );
    sandbox._physics.OnDebugRun ();
}

void BoxStack::UpdateCuboid ( ComponentRef &cuboid, android_vulkan::RigidBodyRef &body ) noexcept
{
    constexpr float const physicsToRender = 32.0F;

    // NOLINTNEXTLINE
    auto& c = *static_cast<StaticMeshComponent*> ( cuboid.get () );

    // NOLINTNEXTLINE
    auto& s = static_cast<android_vulkan::ShapeBox&> ( body->GetShape () );
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
}

} // namespace pbr::physics
