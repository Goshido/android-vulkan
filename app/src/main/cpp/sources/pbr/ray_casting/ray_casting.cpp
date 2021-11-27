#include <pbr/ray_casting/ray_casting.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/opaque_material.h>
#include <pbr/static_mesh_component.h>
#include <shape_box.h>
#include <vertex_info.h>


namespace pbr::ray_casting {

constexpr static float const FIELD_OF_VIEW = 75.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+4F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 100U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 100U;

constexpr float const ROTATION_SPEED = 5.0e-2F * GX_MATH_DOUBLE_PI;

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

    if ( !LoadResources ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    return true;
}

void RayCasting::OnDestroyDevice ( VkDevice device ) noexcept
{
    _body.reset ();
    _cube.reset ();

    _renderSession.OnDestroyDevice ( device );

    _normalTexture->FreeResources ( device );
    _normalTexture.reset ();
    _normalMaterial.reset ();

    _rayTextureHit->FreeResources ( device );
    _rayTextureHit.reset ();

    _rayTextureNoHit->FreeResources ( device );
    _rayTextureNoHit.reset ();
    _rayMaterial.reset ();

    _lineMesh->FreeResources ( device );
    _lineMesh.reset ();

    DestroyCommandPool ( device );

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
}

bool RayCasting::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
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
    _physics.Resume ();
    return true;
}

void RayCasting::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    _physics.Pause ();
    Camera::ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( device );
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

    android_vulkan::RigidBody& b = *_body.get ();
    b.SetVelocityAngular ( a, false );

    GXMat4 transform {};
    constexpr GXVec3 const l ( 0.0F, 0.0F, 0.0F );
    transform.FromFast ( b.GetRotation (), l );

    // NOLINTNEXTLINE
    auto& c = *static_cast<StaticMeshComponent*> ( _cube.get () );

    // NOLINTNEXTLINE
    auto const& boxShape = static_cast<android_vulkan::ShapeBox&> ( b.GetShape() );

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
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &info, nullptr, &_commandPool ),
        "RayCasting::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "RayCasting::_commandPool" )
    return true;
}

void RayCasting::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "RayCasting::_commandPool" )
}

bool RayCasting::LoadResources ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr GXVec3 const cameraLocation ( 9.42572402F, -0.913246631F, 1.12910795F );
    _camera.SetLocation ( cameraLocation );
    _camera.SetRotation ( -0.0880209357F, -1.59671032F );
    _camera.SetMovingSpeed ( 1.67F );
    _camera.Update ( 0.0F );

    constexpr size_t const cubeMeshCommandBuffers = 1U;
    constexpr size_t const cubeMaterialCommandBuffers = 5U;
    constexpr size_t const cubeCommandBuffers = cubeMeshCommandBuffers + cubeMaterialCommandBuffers;

    constexpr size_t const normalMaterialCommandBuffers = 1U;

    constexpr size_t const rayMeshCommandBuffers = 1U;
    constexpr size_t const rayTextureCommandBuffers = 2U;
    constexpr size_t const rayCommandBuffers = rayMeshCommandBuffers + rayTextureCommandBuffers;

    constexpr size_t const comBuffs = cubeCommandBuffers + normalMaterialCommandBuffers + rayCommandBuffers;

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

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, cb ),
        "RayCasting::LoadResources",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    size_t consumed;

    _cube = std::make_shared<StaticMeshComponent> ( renderer,
        consumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/Props/PBR/DefaultCSGEmissiveBright.mtl",
        cb
    );

    if ( !_cube )
        return false;

    cb += consumed;

    GXRandomize ();
    _angular0 = GenerateAngular ();
    _angular1 = GenerateAngular ();
    _angularSlider = 0.0F;

    _body = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& ph = *_body.get ();
    ph.SetLocation ( 0.0F, 0.0F, 0.0F, false );
    ph.EnableKinematic ();
    ph.SetTag ( "Cube" );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( 2.0F, 7.0F, 5.0F );
    ph.SetShape ( shape, false );

    ph.SetVelocityAngular ( _angular0, false );

    if ( !_physics.AddRigidBody ( _body ) )
    {
        android_vulkan::LogError ( "RayCasting::LoadResources - Cant' add rigid body." );
        return false;
    }

    if ( !CreateTexture ( renderer, _rayTextureHit, cb, 115U, 185U, 0U, "rayHit" ) )
        return false;

    if ( !CreateTexture ( renderer, _rayTextureNoHit, cb, 70U, 185U, 225U, "rayNoHit" ) )
        return false;

    if ( !CreateTexture ( renderer, _normalTexture, cb, 220U, 70U, 225U, "normal" ) )
        return false;

    _rayMaterial = std::make_shared<OpaqueMaterial> ();
    _normalMaterial = std::make_shared<OpaqueMaterial> ();

    SwitchEmission ( _normalMaterial, _normalTexture );

    constexpr android_vulkan::VertexInfo const vertices[ 6U ]
    {
        android_vulkan::VertexInfo
        (
            GXVec3 ( 0.998F, 0.0F, 0.0F ),
            GXVec2 ( 0.0F, 1.0F ),
            GXVec3 ( 0.0F, 0.0F, 1.0F ),
            GXVec3 ( 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 0.0F, 1.0F, 0.0F )
        ),

        android_vulkan::VertexInfo
        (
            GXVec3 ( -0.502F, 0.866F, 0.0F ),
            GXVec2 ( 0.5F, 1.0F ),
            GXVec3 ( 0.0F, 0.0F, 1.0F ),
            GXVec3 ( 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 0.0F, 1.0F, 0.0F )
        ),

        android_vulkan::VertexInfo
        (
            GXVec3 ( -0.502F, -0.866F, 0.0F ),
            GXVec2 ( 1.0F, 1.0F ),
            GXVec3 ( 0.0F, 0.0F, 1.0F ),
            GXVec3 ( 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 0.0F, 1.0F, 0.0F )
        ),

        android_vulkan::VertexInfo
        (
            GXVec3 ( 0.998F, 0.0F, 1.0F ),
            GXVec2 ( 0.0F, 0.0F ),
            GXVec3 ( 0.0F, 0.0F, 1.0F ),
            GXVec3 ( 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 0.0F, 1.0F, 0.0F )
        ),

        android_vulkan::VertexInfo
        (
            GXVec3 ( -0.502F, 0.866F, 1.0F ),
            GXVec2 ( 0.5F, 0.0F ),
            GXVec3 ( 0.0F, 0.0F, 1.0F ),
            GXVec3 ( 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 0.0F, 1.0F, 0.0F )
        ),

        android_vulkan::VertexInfo
        (
            GXVec3 ( -0.502F, -0.866F, 1.0F ),
            GXVec2 ( 1.0F, 0.0F ),
            GXVec3 ( 0.0F, 0.0F, 1.0F ),
            GXVec3 ( 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 0.0F, 1.0F, 0.0F )
        )
    };

    constexpr uint32_t const indices[ 18U ] =
    {
        0U, 1U, 4U,
        0U, 4U, 3U,

        1U, 2U, 5U,
        1U, 5U, 4U,

        2U, 0U, 3U,
        2U, 3U, 5U
    };

    GXAABB bounds {};
    bounds.AddVertex ( -0.502F, -0.866F, 0.0F );
    bounds.AddVertex ( 0.998F, 0.866F, 1.0F );

    _lineMesh = std::make_shared<android_vulkan::MeshGeometry> ();
    android_vulkan::MeshGeometry &lineMesh = *_lineMesh.get ();

    result = lineMesh.LoadMesh ( reinterpret_cast<uint8_t const*> ( vertices ),
        sizeof ( vertices ),
        indices,
        static_cast<uint32_t> ( std::size ( indices ) ),
        bounds,
        renderer,
        *cb
    );

    if ( !result )
        return false;

    cb += 1U;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "RayCasting::LoadResources",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    _cube->FreeTransferResources ( device );
    _rayTextureHit->FreeTransferResources ( device );
    _rayTextureNoHit->FreeTransferResources ( device );
    _normalTexture->FreeTransferResources ( device );
    lineMesh.FreeTransferResources ( device );

    return true;
}

void RayCasting::Raycast () noexcept
{
    constexpr GXVec3 const rayFrom ( 7.577F, -5.211F, -6.411F );
    constexpr GXVec3 const rayTo ( -4.267F, 2.92F, 8.586F );

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

    constexpr float const rayThickness = 1.0e-2F;
    x.Multiply ( x, rayThickness );
    y.Multiply ( y, rayThickness );

    basis.SetX ( x );
    basis.SetY ( y );
    basis.SetZ ( dir );

    GXMat4 transform {};
    transform.From ( basis, rayFrom );

    GXAABB bounds {};
    _lineMesh->GetBounds ().Transform ( bounds, transform );

    constexpr android_vulkan::Half4 const color {};
    android_vulkan::RaycastResult result {};

    if ( !_physics.Raycast ( result, rayFrom, rayTo ) )
    {
        SwitchEmission ( _rayMaterial, _rayTextureNoHit );
        _renderSession.SubmitMesh ( _lineMesh, _rayMaterial, transform, bounds, color, color, color, color );
        return;
    }

    SwitchEmission ( _rayMaterial, _rayTextureHit );
    _renderSession.SubmitMesh ( _lineMesh, _rayMaterial, transform, bounds, color, color, color, color );

    basis.From ( result._normal );

    basis.GetX ( x );
    basis.GetY ( y );

    constexpr float const normalThickness = 2.0e-2F;
    x.Multiply ( x, normalThickness );

    GXVec3 z {};
    basis.GetZ ( z );

    y.Multiply ( y, normalThickness );
    basis.SetX ( x );
    basis.SetY ( y );

    constexpr float const normalLength = 7.777e-1F;
    z.Multiply ( z, normalLength );
    basis.SetZ ( z );

    transform.From ( basis, result._point );
    _lineMesh->GetBounds ().Transform ( bounds, transform );

    _renderSession.SubmitMesh ( _lineMesh, _normalMaterial, transform, bounds, color, color, color, color );
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
        *commandBuffers
    );

    if ( !result )
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

    if ( std::isnan ( v._data[ 0U ] ) )
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
    // NOLINTNEXTLINE
    auto& m = *static_cast<OpaqueMaterial*> ( material.get () );
    m.SetEmission ( emission );
}

} // namespace pbr::ray_casting
