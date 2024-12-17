#include <precompiled_headers.hpp>
#include <pbr/collision/collision.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/material_manager.hpp>
#include <pbr/mesh_manager.hpp>
#include <pbr/static_mesh_component.hpp>
#include <shape_box.hpp>


namespace pbr::collision {

namespace {

constexpr float FIELD_OF_VIEW = 75.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+4F;

constexpr uint32_t RESOLUTION_SCALE_WIDTH = 100U;
constexpr uint32_t RESOLUTION_SCALE_HEIGHT = 100U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool Collision::IsReady () noexcept
{
    return static_cast<bool> ( _contactMesh );
}

bool Collision::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );

    _camera.Update ( dt );
    GXMat4 const &cameraLocal = _camera.GetLocalMatrix ();
    _renderSession.Begin ( cameraLocal, _camera.GetProjectionMatrix () );

    _manipulator.Update ( cameraLocal, dt );

    _contactManager.Reset ();
    _contactDetector.Check ( _contactManager, _cubes[ 0U ]._body, _cubes[ 1U ]._body );

    // NOLINTNEXTLINE - downcast.
    auto &light = *static_cast<PointLightComponent*> ( _cameraLight.get () );

    GXVec3 lightLocation {};
    cameraLocal.GetW ( lightLocation );
    light.SetLocation ( lightLocation );
    light.Submit ( _renderSession );

    for ( auto &cube : _cubes )
    {
        UpdateCuboid ( cube );

        // NOLINTNEXTLINE - downcast.
        auto &renderableComponent = static_cast<RenderableComponent &> ( *cube._component );
        renderableComponent.Submit ( _renderSession );
    }

    GXMat4 transform {};
    constexpr float sphereSize = 0.02F * UNITS_IN_METER;
    constexpr GXVec3 sphereDims ( sphereSize * 0.5F, sphereSize * 0.5F, sphereSize * 0.5F );
    transform.Scale ( sphereSize, sphereSize, sphereSize );

    auto const submit = [ & ] ( GXVec3 const &loc, GXColorUNORM color ) noexcept {
        GXVec3 location {};
        location.Multiply ( loc, UNITS_IN_METER );
        transform.SetW ( location );

        GXAABB bounds {};
        GXVec3 v {};
        v.Sum ( location, sphereDims );
        bounds.AddVertex ( v );

        v.Subtract ( location, sphereDims );
        bounds.AddVertex ( v );

        _renderSession.SubmitMesh ( _contactMesh,
            _contactMaterial,
            transform,
            bounds,
            GeometryPassProgram::ColorData ( color, _defaultColor, _defaultColor, _defaultColor, 1.0F )
        );
    };

    for ( auto const &manifold : _contactManager.GetContactManifolds () )
    {
        android_vulkan::Contact const* contacts = manifold._contacts;
        size_t const count = manifold._contactCount;

        for ( size_t i = 0U; i < count; ++i )
        {
            android_vulkan::Contact const &contact = contacts[ i ];
            submit ( contact._pointA, _aColor );
            submit ( contact._pointB, _bColor );
        }
    }

    return _renderSession.End ( renderer, deltaTime );
}

bool Collision::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !CreateCommandPool ( renderer ) || !_renderSession.OnInitDevice ( renderer ) || !CreateScene ( renderer ) )
        return false;

    _renderSession.FreeTransferResources ( renderer );
    return true;
}

void Collision::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    DestroyScene ( renderer );
    _renderSession.OnDestroyDevice ( renderer );
    DestroyCommandPool ( device );
}

bool Collision::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
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

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution ) ) [[unlikely]]
        return false;

    _camera.CaptureInput ();
    _manipulator.Capture ( _cubes[ 1U ]._body );
    return true;
}

void Collision::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    _manipulator.Free ();
    _camera.ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
}

bool Collision::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &poolInfo, nullptr, &_commandPool ),
        "Collision::OnInitDevice",
        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _commandPool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "pbr::collision::Collision::_commandPool"
    )

    return true;
}

void Collision::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
}

bool Collision::CreateScene ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.SetLocation ( GXVec3 ( 0.77F, 20.8F, -55.4F ) );
    _camera.Update ( 0.0F );

    constexpr size_t cubeBuffers = 1U;
    constexpr size_t defaultMaterialBuffers = 5U;
    constexpr size_t sphereBuffers = 1U;
    constexpr size_t unlitMaterialBuffers = 5U;
    constexpr size_t totalBuffers = cubeBuffers + defaultMaterialBuffers + sphereBuffers + unlitMaterialBuffers;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( totalBuffers )
    };

    _commandBuffers.resize ( totalBuffers );
    VkCommandBuffer* commandBuffers = _commandBuffers.data ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, commandBuffers ),
        "Collision::CreateScene",
        "Can't allocate command buffers"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < totalBuffers; ++i )
    AV_SET_VULKAN_OBJECT_NAME ( device, commandBuffers[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "Asset #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    size_t consumed = 0U;
    _cubes.resize ( 2U );

    result = AppendCuboid ( renderer,
        commandBuffers,
        consumed,
        "Floor",
        _cubes[ 0U ]._component,
        "pbr/assets/Props/PBR/DefaultCSGEmissive.mtl",
        _defaultColor,
        _cubes[ 0U ]._body,
        0.0F,
        -0.25F,
        0.0F,
        32.0F,
        0.5F,
        32.0F
    );

    if ( !result ) [[unlikely]]
        return false;

    commandBuffers += consumed;

    result = AppendCuboid ( renderer,
        commandBuffers,
        consumed,
        "Cube #0",
        _cubes[ 1U ]._component,
        "pbr/assets/System/Default.mtl",
        GXColorUNORM ( 99U, 211U, 222U, 255U ),
        _cubes[ 1U ]._body,
        0.0F,
        0.15F,
        0.0F,
        0.6F,
        0.3F,
        0.7F
    );

    if ( !result ) [[unlikely]]
        return false;

    commandBuffers += consumed;

    _contactMesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        "pbr/system/unit-sphere.mesh2",
        *commandBuffers,
        VK_NULL_HANDLE
    );

    if ( !_contactMesh ) [[unlikely]]
        return false;

    consumed += consumed;

    _contactMaterial = MaterialManager::GetInstance ().LoadMaterial ( renderer,
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

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "Collision::CreateScene",
        "Can't run upload commands"
    );

    if ( !result ) [[unlikely]]
        return false;

    for ( auto &cube : _cubes )
    {
        // NOLINTNEXTLINE - downcast.
        auto &renderableComponent = static_cast<RenderableComponent &> ( *cube._component );
        renderableComponent.FreeTransferResources ( renderer );
    }

    _contactMesh->FreeTransferResources ( renderer );

    // NOLINTNEXTLINE - downcast.
    auto &m = static_cast<GeometryPassMaterial &> ( *_contactMaterial );

    if ( m.GetAlbedo () )
        m.GetAlbedo ()->FreeTransferResources ( renderer );

    if ( m.GetEmission () )
        m.GetEmission ()->FreeTransferResources ( renderer );

    if ( m.GetNormal () )
        m.GetNormal ()->FreeTransferResources ( renderer );

    if ( m.GetParam () )
        m.GetParam ()->FreeTransferResources ( renderer );

    MaterialManager::GetInstance ().FreeTransferResources ( renderer );
    return true;
}

void Collision::DestroyScene ( android_vulkan::Renderer &renderer ) noexcept
{
    _cubes.clear ();
    _cubes.shrink_to_fit ();

    _cubes.clear ();
    _cubes.shrink_to_fit ();

    _cameraLight = nullptr;

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
}

bool Collision::AppendCuboid ( android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers,
    size_t &commandBufferConsumed,
    std::string &&tag,
    ComponentRef &visual,
    char const* material,
    GXColorUNORM color,
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

    physical = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody &ph = *physical.get ();
    ph.SetLocation ( x, y, z, false );
    ph.EnableKinematic ();
    ph.EnableSleep ();
    ph.SetTag ( std::move ( tag ) );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( w, h, d );
    ph.SetShape ( shape, false );
    return true;
}

void Collision::UpdateCuboid ( CubeInfo &cube ) noexcept
{
    // NOLINTNEXTLINE
    auto &c = *static_cast<StaticMeshComponent*> ( cube._component.get () );

    // NOLINTNEXTLINE
    auto &s = static_cast<android_vulkan::ShapeBox &> ( cube._body->GetShape () );
    GXMat4 transform = cube._body->GetTransform ();

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

} // namespace pbr::collision
