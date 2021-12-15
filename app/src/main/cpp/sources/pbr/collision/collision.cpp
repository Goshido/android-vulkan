#include <pbr/collision/collision.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/static_mesh_component.h>
#include <shape_box.h>


namespace pbr::collision {

constexpr static float const FIELD_OF_VIEW = 75.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+4F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 100U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 100U;

bool Collision::IsReady () noexcept
{
    return static_cast<bool> ( _contactMesh );
}

bool Collision::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );

    _camera.Update ( dt );
    GXMat4 const& cameraLocal = _camera.GetLocalMatrix ();
    _renderSession.Begin ( cameraLocal, _camera.GetProjectionMatrix () );

    _manipulator.Update ( cameraLocal, dt );

    _contactManager.Reset ();
    _contactDetector.Check ( _contactManager, _cubes[ 0U ]._body, _cubes[ 1U ]._body );

    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLightComponent*> ( _cameraLight.get () );

    GXVec3 lightLocation {};
    cameraLocal.GetW ( lightLocation );
    light.SetLocation ( lightLocation );
    _cameraLight->Submit ( _renderSession );

    for ( auto& cube : _cubes )
    {
        UpdateCuboid ( cube );
        cube._component->Submit ( _renderSession );
    }

    GXMat4 transform {};
    constexpr float const rendererScale = 32.0F;
    constexpr float const sphereSize = 0.02F * rendererScale;
    constexpr GXVec3 const sphereDims ( sphereSize * 0.5F, sphereSize * 0.5F, sphereSize * 0.5F );
    transform.Scale ( sphereSize, sphereSize, sphereSize );

    auto submit = [ & ] ( GXVec3 const &loc, GXColorRGB const &color ) noexcept {
        GXVec3 location {};
        location.Multiply ( loc, rendererScale );
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
            color,
            _defaultColor,
            _defaultColor,
            _defaultColor
        );
    };

    for ( auto const& manifold : _contactManager.GetContactManifolds () )
    {
        android_vulkan::Contact const* contacts = manifold._contacts;
        size_t const count = manifold._contactCount;

        for ( size_t i = 0U; i < count; ++i )
        {
            android_vulkan::Contact const& contact = contacts[ i ];
            submit ( contact._pointA, _aColor );
            submit ( contact._pointB, _bColor );
        }
    }

    return _renderSession.End ( renderer, deltaTime );
}

bool Collision::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !CreateCommandPool ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    if ( !_renderSession.OnInitDevice ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    if ( !CreateScene ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    return true;
}

void Collision::OnDestroyDevice ( VkDevice device ) noexcept
{
    DestroyScene ( device );
    _renderSession.OnDestroyDevice ( device );
    DestroyCommandPool ( device );
}

bool Collision::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
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
    _manipulator.Capture ( _cubes[ 1U ]._body );
    return true;
}

void Collision::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    _manipulator.Free ();
    Camera::ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( device );
}

bool Collision::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &poolInfo, nullptr, &_commandPool ),
        "Collision::OnInitDevice",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "Collision::_commandPool" )
    return true;
}

void Collision::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "Collision::_commandPool" )
}

bool Collision::CreateScene ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.SetLocation ( GXVec3 ( 0.77F, 20.8F, -55.4F ) );
    _camera.Update ( 0.0F );

    constexpr size_t const cubeBuffers = 1U;
    constexpr size_t const defaultMaterialBuffers = 5U;
    constexpr size_t const sphereBuffers = 1U;
    constexpr size_t const unlitMaterialBuffers = 5U;
    constexpr size_t const totalBuffers = cubeBuffers + defaultMaterialBuffers + sphereBuffers + unlitMaterialBuffers;

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
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers ),
        "Collision::CreateScene",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

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

    if ( !result )
        return false;

    commandBuffers += consumed;

    result = AppendCuboid ( renderer,
        commandBuffers,
        consumed,
        "Cube #0",
        _cubes[ 1U ]._component,
        "pbr/assets/System/Default.mtl",

        GXColorRGB ( static_cast<GXUByte> ( 99U ),
            static_cast<GXUByte> ( 211U ),
            static_cast<GXUByte> ( 222U ),
            static_cast<GXUByte> ( 255U )
        ),

        _cubes[ 1U ]._body,
        0.0F,
        0.15F,
        0.0F,
        0.6F,
        0.3F,
        0.7F
    );

    if ( !result )
        return false;

    commandBuffers += consumed;

    _contactMesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        "pbr/system/unit-sphere.mesh2",
        *commandBuffers
    );

    if ( !_contactMesh )
        return false;

    consumed += consumed;

    _contactMaterial = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        consumed,
        "pbr/assets/System/Default.mtl",
        commandBuffers
    );

    _cameraLight = std::make_shared<PointLightComponent> ();

    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLightComponent*> ( _cameraLight.get () );
    light.SetIntensity ( 16.0F );
    light.SetBoundDimensions ( 1600.0F, 1600.0F, 1600.0F );

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "Collision::CreateScene",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto& cube : _cubes )
        cube._component->FreeTransferResources ( device );

    _contactMesh->FreeTransferResources ( device );

    // Note it's safe to cast like that here. "NOLINT" is clang-tidy control comment.
    auto* m = static_cast<OpaqueMaterial*> ( _contactMaterial.get () ); // NOLINT

    if ( m->GetAlbedo () )
        m->GetAlbedo ()->FreeTransferResources ( device );

    if ( m->GetEmission () )
        m->GetEmission ()->FreeTransferResources ( device );

    if ( m->GetNormal () )
        m->GetNormal ()->FreeTransferResources ( device );

    if ( m->GetParam () )
        m->GetParam ()->FreeTransferResources ( device );

    return true;
}

void Collision::DestroyScene ( VkDevice device ) noexcept
{
    _cubes.clear ();
    _cubes.shrink_to_fit ();

    _cubes.clear ();
    _cubes.shrink_to_fit ();

    _cameraLight = nullptr;

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
}

bool Collision::AppendCuboid ( android_vulkan::Renderer &renderer,
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

    physical = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& ph = *physical.get ();
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
    constexpr float const physicsToRender = 32.0F;

    // NOLINTNEXTLINE
    auto& c = *static_cast<StaticMeshComponent*> ( cube._component.get () );

    // NOLINTNEXTLINE
    auto& s = static_cast<android_vulkan::ShapeBox&> ( cube._body->GetShape () );
    GXMat4 transform = cube._body->GetTransform ();

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

} // namespace pbr::collision
