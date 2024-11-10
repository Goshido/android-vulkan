#include <precompiled_headers.hpp>
#include <pbr/box_stack/box_stack.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/component.hpp>
#include <pbr/material_manager.hpp>
#include <pbr/mesh_manager.hpp>
#include <pbr/static_mesh_component.hpp>
#include <gamepad.hpp>
#include <global_force_gravity.hpp>
#include <logger.hpp>
#include <shape_box.hpp>
#include <shape_sphere.hpp>


namespace pbr::box_stack {

namespace {

constexpr float FIELD_OF_VIEW = 75.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+4F;

constexpr uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

constexpr GXVec3 FREE_FALL_ACCELERATION ( 0.0F, -9.81F, 0.0F );
constexpr float TIME_SPEED = 1.0F;

constexpr size_t CUBES = 6U;

} // end of anonymous namespace

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
    GXMat4 const &cameraLocal = _camera.GetLocalMatrix ();
    _renderSession.Begin ( cameraLocal, _camera.GetProjectionMatrix () );

    // NOLINTNEXTLINE - downcast.
    auto &light = static_cast<PointLightComponent &> ( *_cameraLight );

    GXVec3 lightLocation {};
    cameraLocal.GetW ( lightLocation );
    light.SetLocation ( lightLocation );

    GXMat4 transform {};
    constexpr float sphereSize = 0.02F * UNITS_IN_METER;
    constexpr GXVec3 sphereDims ( sphereSize * 0.5F, sphereSize * 0.5F, sphereSize * 0.5F );
    transform.Scale ( sphereSize, sphereSize, sphereSize );

    auto submit = [ & ] ( GXVec3 const &loc, GXColorRGB const &color ) noexcept {
        GXVec3 location {};
        location.Multiply ( loc, UNITS_IN_METER );
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

    for ( auto const &manifold : _physics.GetContactManifolds () )
    {
        android_vulkan::Contact* contact = manifold._contacts;

        for ( size_t i = 0U; i < manifold._contactCount; ++i )
        {
            android_vulkan::Contact const &c = contact[ i ];
            submit ( c._pointA, _colors[ 0U ] );
            submit ( c._pointB, _colors[ 2U ] );
        }
    }

    for ( auto &component : _components )
    {
        // NOLINTNEXTLINE - downcast.
        auto &renderableComponent = static_cast<RenderableComponent &> ( *component );
        renderableComponent.Submit ( _renderSession );
    }

    return _renderSession.End ( renderer, deltaTime );
}

bool BoxStack::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &createInfo, nullptr, &_commandPool ),
        "pbr::box_stack::BoxStack::OnInit",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _commandPool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "pbr::box_stack::BoxStack::_commandPool"
    )

    if ( !_renderSession.OnInitDevice ( renderer ) )
        return false;

    InitColors ();

    if ( !CreateSceneManual ( renderer ) )
        return false;

    _renderSession.FreeTransferResources ( renderer );
    return true;
}

void BoxStack::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    DestroyPhysics ();
    _cubes.clear ();
    _components.clear ();
    _renderSession.OnDestroyDevice ( renderer );

    DestroyCommandPool ( renderer.GetDevice () );
    _physics.Reset ();

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
}

bool BoxStack::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const &surfaceResolution = renderer.GetViewportResolution ();

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

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution ) )
        return false;

    _camera.CaptureInput ();

    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

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

void BoxStack::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::LeftBumper, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::RightBumper, android_vulkan::eButtonState::Down );

    _physics.Pause ();

    _camera.ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
}

bool BoxStack::AppendCuboid ( android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers,
    size_t &commandBufferConsumed,
    std::string &&tag,
    ComponentRef &visual,
    char const* material,
    GXColorRGB const &color,
    android_vulkan::RigidBodyRef &physical,
    float x,
    float y,
    float z,
    float w,
    float h,
    float d
) noexcept
{
    bool success;

    visual = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        "pbr/system/unit-cube.mesh2",
        material,
        commandBuffers,
        nullptr,
        "Mesh"
    );

    if ( !success )
    {
        visual.reset ();
        return false;
    }

    // NOLINTNEXTLINE
    auto &v = *static_cast<StaticMeshComponent*> ( visual.get () );
    v.SetColor0 ( color );

    _components.push_back ( visual );

    physical = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody &ph = *physical.get ();
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

    android_vulkan::LogError ( "pbr::box_stack::BoxStack::AppendCuboid - Can't add rigid body." );
    return false;
}

void BoxStack::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
}

bool BoxStack::CreateSceneManual ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.SetLocation ( GXVec3 ( 0.77F, 35.8F, -55.4F ) );
    _camera.Update ( 0.0F );

    _physics.SetTimeSpeed ( TIME_SPEED );

    if ( !_physics.AddGlobalForce ( std::make_shared<android_vulkan::GlobalForceGravity> ( FREE_FALL_ACCELERATION ) ) )
    {
        [[unlikely]]
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

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, commandBuffers ),
        "pbr::box_stack::BoxStack::CreateSceneManual",
        "Can't allocate command buffers"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < comBuffs; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, commandBuffers[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "Asset #%zu", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    size_t consumed = 0U;
    _defaultColor.From ( 255U, 255U, 255U, 255U );

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

    if ( !result ) [[unlikely]]
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

        if ( !result ) [[unlikely]]
            return false;

        commandBuffers += consumed;

        android_vulkan::RigidBody &b = *_cubeBodies[ i ].get ();
        b.SetMass ( 7.77F, true );
    }

    _sphereMesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        "pbr/system/unit-sphere.mesh2",
        *commandBuffers,
        VK_NULL_HANDLE
    );

    commandBuffers += consumed;

    _sphereMaterial = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        consumed,
        "pbr/assets/System/Default.mtl",
        commandBuffers,
        nullptr
    );

    _cameraLight = std::make_shared<PointLightComponent> ();

    // NOLINTNEXTLINE
    auto &light = *static_cast<PointLightComponent*> ( _cameraLight.get () );
    light.SetIntensity ( 16.0F );
    light.SetBoundDimensions ( 1600.0F, 1600.0F, 1600.0F );
    _components.push_back ( _cameraLight );

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "pbr::box_stack::BoxStack::CreateSceneManual",
        "Can't run upload commands"
    );

    if ( !result ) [[unlikely]]
        return false;

    for ( auto &component : _components )
    {
        // NOLINTNEXTLINE - downcast.
        auto &renderableComponent = static_cast<RenderableComponent &> ( *component );
        renderableComponent.FreeTransferResources ( renderer );
    }

    MaterialManager::GetInstance ().FreeTransferResources ( renderer );
    _sphereMesh->FreeTransferResources ( renderer );
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
    _colors[ 0U ].From ( 115U, 185U, 0U, 255U );

    // Yellow
    _colors[ 1U ].From ( 223U, 202U, 79U, 255U );

    // Red
    _colors[ 2U ].From ( 223U, 79U, 88U, 255U );

    // Purple
    _colors[ 3U ].From ( 223U, 79U, 210U, 255U );

    // Blue
    _colors[ 4U ].From ( 100U, 79U, 223U, 255U );

    // Cyan
    _colors[ 5U ].From ( 79U, 212U, 223U, 255U );

    // Green
    _colors[ 6U ].From ( 79U, 223U, 107U, 255U );
}

void BoxStack::OnLeftBumper ( void* context ) noexcept
{
    auto &sandbox = *static_cast<BoxStack*> ( context );
    android_vulkan::Physics &p = sandbox._physics;

    if ( p.IsPaused () )
    {
        p.Resume ();
        return;
    }

    p.Pause ();
}

void BoxStack::OnRightBumper ( void* context ) noexcept
{
    auto &sandbox = *static_cast<BoxStack*> ( context );
    sandbox._physics.OnDebugRun ();
}

void BoxStack::UpdateCuboid ( ComponentRef &cuboid, android_vulkan::RigidBodyRef &body ) noexcept
{
    // NOLINTNEXTLINE
    auto &c = *static_cast<StaticMeshComponent*> ( cuboid.get () );

    // NOLINTNEXTLINE
    auto &s = static_cast<android_vulkan::ShapeBox &> ( body->GetShape () );
    GXMat4 transform = body->GetTransform ();

    GXVec3 dims ( s.GetWidth (), s.GetHeight (), s.GetDepth () );
    dims.Multiply ( dims, UNITS_IN_METER );

    GXMat4 scale {};
    scale.Scale ( dims._data[ 0U ], dims._data[ 1U ], dims._data[ 2U ] );

    GXVec3 location {};
    transform.GetW ( location );
    location.Multiply ( location, UNITS_IN_METER );
    transform.SetW ( location );

    GXMat4 resultTransform {};
    resultTransform.Multiply ( scale, transform );

    c.SetTransform ( resultTransform );
}

} // namespace pbr::physics
