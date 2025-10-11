#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <platform/android/pbr/ray_casting/ray_casting.hpp>
#include <platform/android/pbr/material_manager.hpp>
#include <platform/android/pbr/mesh_manager.hpp>
#include <platform/android/pbr/opaque_material.hpp>
#include <platform/android/pbr/static_mesh_component.hpp>
#include <shape_box.hpp>
#include <vertex_info.hpp>


namespace pbr::ray_casting {

namespace {

constexpr float FIELD_OF_VIEW = 75.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+4F;

constexpr uint32_t RESOLUTION_SCALE_WIDTH = 100U;
constexpr uint32_t RESOLUTION_SCALE_HEIGHT = 100U;

constexpr float ROTATION_SPEED = 5.0e-2F * GX_MATH_DOUBLE_PI;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool RayCasting::IsReady () noexcept
{
    return static_cast<bool> ( _cube );
}

bool RayCasting::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );

    _camera.Update ( dt );
    _renderSession.Begin ( _camera.GetLocalMatrix (), _camera.GetProjectionMatrix () );

    Animate ( dt );
    Raycast ();

    return _renderSession.End ( renderer, deltaTime );
}

bool RayCasting::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !CreateCommandPool ( renderer ) || !_renderSession.OnInitDevice ( renderer ) || !LoadResources ( renderer ) )
    {
        [[unlikely]]
        return false;
    }

    _renderSession.FreeTransferResources ( renderer );
    return true;
}

void RayCasting::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _body.reset ();
    _cube.reset ();

    _renderSession.OnDestroyDevice ( renderer );

    _normalTexture->FreeResources ( renderer );
    _normalTexture.reset ();
    _normalMaterial.reset ();

    _rayTextureHit->FreeResources ( renderer );
    _rayTextureHit.reset ();

    _rayTextureNoHit->FreeResources ( renderer );
    _rayTextureNoHit.reset ();
    _rayMaterial.reset ();

    _lineMesh->FreeResources ( renderer );
    _lineMesh.reset ();

    DestroyCommandPool ( renderer.GetDevice () );
    _physics.Reset ();

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
}

bool RayCasting::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
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
    _physics.Resume ();
    return true;
}

void RayCasting::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    _physics.Pause ();
    _camera.ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
}

void RayCasting::Animate ( float deltaTime ) noexcept
{
    _physics.Simulate ( deltaTime );
    _angularSlider += deltaTime * 0.5F;

    if ( _angularSlider > 1.0F )
    {
        while ( _angularSlider > 1.0F )
            _angularSlider -= 1.0F;

        _angular0 = _angular1;
        _angular1 = GenerateAngular ();
    }

    GXVec3 a {};
    a.LinearInterpolation ( _angular0, _angular1, _angularSlider );

    android_vulkan::RigidBody &b = *_body.get ();
    b.SetVelocityAngular ( a, false );

    GXMat4 transform {};
    constexpr GXVec3 l ( 0.0F, 0.0F, 0.0F );
    transform.FromFast ( b.GetRotation (), l );

    // NOLINTNEXTLINE - downcast.
    auto &c = static_cast<StaticMeshComponent &> ( *_cube );

    // NOLINTNEXTLINE - downcast.
    auto const &boxShape = static_cast<android_vulkan::ShapeBox &> ( b.GetShape () );

    GXMat4 local {};
    local.Scale ( boxShape.GetWidth (), boxShape.GetHeight (), boxShape.GetDepth () );

    GXMat4 t {};
    t.Multiply ( local, transform );

    c.SetTransform ( t );
    c.Submit ( _renderSession );
}

bool RayCasting::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &info, nullptr, &_commandPool ),
        "pbr::ray_casting::RayCasting::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _commandPool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "pbr::ray_casting::RayCasting::_commandPool"
    )

    return true;
}

void RayCasting::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
}

bool RayCasting::LoadResources ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr GXVec3 const cameraLocation ( 9.42572402F, -0.913246631F, 1.12910795F );
    _camera.SetLocation ( cameraLocation );
    _camera.SetRotation ( -0.0880209357F, -1.59671032F );
    _camera.SetMovingSpeed ( 1.67F );
    _camera.Update ( 0.0F );

    constexpr size_t cubeCommandBuffers = 5U;
    constexpr size_t normalMaterialCommandBuffers = 1U;
    constexpr size_t rayCommandBuffers = 2U;
    constexpr size_t comBuffs = cubeCommandBuffers + normalMaterialCommandBuffers + rayCommandBuffers;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( comBuffs )
    };

    std::array<VkCommandBuffer, comBuffs> commandBuffers {};
    VkCommandBuffer* cb = commandBuffers.data ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, cb ),
        "RayCasting::LoadResources",
        "Can't allocate command buffers"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < comBuffs; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, cb[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "Asset #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    std::vector<VkFence> const fences ( comBuffs, VK_NULL_HANDLE );

    size_t consumed;
    bool success;

    _cube = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        consumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/Props/PBR/DefaultCSGEmissiveBright.mtl",
        cb,
        fences.data (),
        "Mesh"
    );

    if ( !success ) [[unlikely]]
    {
        _cube.reset ();
        return false;
    }

    cb += consumed;

    GXRandomize ();
    _angular0 = GenerateAngular ();
    _angular1 = GenerateAngular ();
    _angularSlider = 0.0F;

    _body = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody &ph = *_body.get ();
    ph.SetLocation ( 0.0F, 0.0F, 0.0F, false );
    ph.EnableKinematic ();
    ph.SetTag ( "Cube" );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( 2.0F, 7.0F, 5.0F );
    ph.SetShape ( shape, false );

    ph.SetVelocityAngular ( _angular0, false );

    if ( !_physics.AddRigidBody ( _body ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "RayCasting::LoadResources - Cant' add rigid body." );
        return false;
    }

    if ( !CreateTexture ( renderer, _rayTextureHit, cb, 115U, 185U, 0U, "rayHit" ) ) [[unlikely]]
        return false;

    if ( !CreateTexture ( renderer, _rayTextureNoHit, cb, 70U, 185U, 225U, "rayNoHit" ) ) [[unlikely]]
        return false;

    if ( !CreateTexture ( renderer, _normalTexture, cb, 220U, 70U, 225U, "normal" ) ) [[unlikely]]
        return false;

    _rayMaterial = std::make_shared<OpaqueMaterial> ();
    _normalMaterial = std::make_shared<OpaqueMaterial> ();

    SwitchEmission ( _normalMaterial, _normalTexture );

    constexpr uint32_t const indices[]
    {
        0U, 1U, 4U,
        0U, 4U, 3U,

        1U, 2U, 5U,
        1U, 5U, 4U,

        2U, 0U, 3U,
        2U, 3U, 5U
    };

    constexpr GXVec3 const positions[]
    {
        { 0.998F, 0.0F, 0.0F },
        { -0.502F, 0.866F, 0.0F },
        { -0.502F, -0.866F, 0.0F },
        { 0.998F, 0.0F, 1.0F },
        { -0.502F, 0.866F, 1.0F },
        { -0.502F, -0.866F, 1.0F }
    };

    android_vulkan::VertexInfo const vertices[]
    {
        {
            ._uv { 0.0F, 1.0F },
            ._tbn = android_vulkan::VertexInfo::IDENTITY_TBN
        },
        {
            ._uv { 0.5F, 1.0F },
            ._tbn = android_vulkan::VertexInfo::IDENTITY_TBN
        },
        {
            ._uv { 1.0F, 1.0F },
            ._tbn = android_vulkan::VertexInfo::IDENTITY_TBN
        },
        {
            ._uv { 0.0F, 0.0F },
            ._tbn = android_vulkan::VertexInfo::IDENTITY_TBN
        },
        {
            ._uv { 0.5F, 0.0F },
            ._tbn = android_vulkan::VertexInfo::IDENTITY_TBN
        },
        {
            ._uv { 1.0F, 0.0F },
            ._tbn = android_vulkan::VertexInfo::IDENTITY_TBN
        }
    };

    GXAABB bounds {};
    bounds.AddVertex ( -0.502F, -0.866F, 0.0F );
    bounds.AddVertex ( 0.998F, 0.866F, 1.0F );

    _lineMesh = std::make_shared<android_vulkan::MeshGeometry> ();
    android_vulkan::MeshGeometry &lineMesh = *_lineMesh.get ();

    result = lineMesh.LoadMesh ( renderer,
        { indices, std::size ( indices ) },
        { positions, std::size ( positions ) },
        { vertices, std::size ( vertices ) },
        bounds
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "pbr::ray_casting::RayCasting::LoadResources",
        "Can't run upload commands"
    );

    if ( !result ) [[unlikely]]
        return false;

    // NOLINTNEXTLINE - downcast.
    auto &cube = static_cast<StaticMeshComponent &> ( *_cube );
    cube.FreeTransferResources ( renderer );

    _rayTextureHit->FreeTransferResources ( renderer );
    _rayTextureNoHit->FreeTransferResources ( renderer );
    _normalTexture->FreeTransferResources ( renderer );
    MaterialManager::GetInstance ().FreeTransferResources ( renderer );

    return true;
}

void RayCasting::Raycast () noexcept
{
    constexpr GXVec3 rayFrom ( 7.577F, -5.211F, -6.411F );
    constexpr GXVec3 rayTo ( -4.267F, 2.92F, 8.586F );

    GXVec3 dir {};
    dir.Subtract ( rayTo, rayFrom );

    GXVec3 axis ( dir );
    axis.Normalize ();

    GXMat3 basis {};
    basis.From ( axis );

    GXVec3 x {};
    basis.GetX ( x );

    GXVec3 y {};
    basis.GetY ( y );

    constexpr float rayThickness = 1.0e-2F;
    x.Multiply ( x, rayThickness );
    y.Multiply ( y, rayThickness );

    basis.SetX ( x );
    basis.SetY ( y );
    basis.SetZ ( dir );

    GXMat4 transform {};
    transform.From ( basis, rayFrom );

    GXAABB bounds {};
    _lineMesh->GetBounds ().Transform ( bounds, transform );

    constexpr GXColorUNORM color ( 255U, 255U, 255U, 255U );
    android_vulkan::RaycastResult result {};
    constexpr uint32_t groups = std::numeric_limits<uint32_t>::max ();

    GeometryPassProgram::ColorData const colorData ( color, color, color, color, 1.0F );

    if ( !_physics.Raycast ( result, rayFrom, rayTo, groups ) )
    {
        SwitchEmission ( _rayMaterial, _rayTextureNoHit );
        _renderSession.SubmitMesh ( _lineMesh, _rayMaterial, transform, bounds, colorData );
        return;
    }

    SwitchEmission ( _rayMaterial, _rayTextureHit );
    _renderSession.SubmitMesh ( _lineMesh, _rayMaterial, transform, bounds, colorData );

    basis.From ( result._normal );

    basis.GetX ( x );
    basis.GetY ( y );

    constexpr float normalThickness = 2.0e-2F;
    x.Multiply ( x, normalThickness );

    GXVec3 z {};
    basis.GetZ ( z );

    y.Multiply ( y, normalThickness );
    basis.SetX ( x );
    basis.SetY ( y );

    constexpr float normalLength = 7.777e-1F;
    z.Multiply ( z, normalLength );
    basis.SetZ ( z );

    transform.From ( basis, result._point );
    _lineMesh->GetBounds ().Transform ( bounds, transform );

    _renderSession.SubmitMesh ( _lineMesh, _normalMaterial, transform, bounds, colorData );
}

bool RayCasting::CreateTexture ( android_vulkan::Renderer &renderer,
    Texture2DRef &texture,
    VkCommandBuffer* &commandBuffers,
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    std::string &&name
) noexcept
{
    uint8_t const color[ 4U ] = { red, green, blue, 255U };
    texture = std::make_shared<android_vulkan::Texture2D> ();

    bool const result = texture->UploadData ( renderer,
        color,
        sizeof ( color ),

        VkExtent2D
        {
            .width = 1U,
            .height = 1U
        },

        VK_FORMAT_R8G8B8A8_SRGB,
        false,
        *commandBuffers,
        false,
        VK_NULL_HANDLE
    );

    if ( !result ) [[unlikely]]
        return false;

    commandBuffers += 1U;
    texture->AssignName ( std::move ( name ) );

    return true;
}

GXVec3 RayCasting::GenerateAngular () noexcept
{
    GXVec3 v ( GXRandomBetween ( -1.0F, 1.0F ),
        GXRandomBetween ( -1.0F, 1.0F ),
        GXRandomBetween ( -1.0F, 1.0F )
    );

    v.Normalize ();

    if ( std::isnan ( v._data[ 0U ] ) ) [[unlikely]]
    {
        // Vector is too small to normalize. Re-rolling with fixing of x component.
        v._data[ 0U ] = 1.0F;
        v._data[ 1U ] = GXRandomBetween ( -1.0F, 1.0F );
        v._data[ 2U ] = GXRandomBetween ( -1.0F, 1.0F );
        v.Normalize ();
    }

    v.Multiply ( v, ROTATION_SPEED );
    return v;
}

void RayCasting::SwitchEmission ( MaterialRef &material, Texture2DRef &emission ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &m = static_cast<OpaqueMaterial &> ( *material );
    m.SetEmission ( emission );
}

} // namespace pbr::ray_casting
