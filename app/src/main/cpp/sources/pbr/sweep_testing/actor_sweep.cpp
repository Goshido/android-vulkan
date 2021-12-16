#include <pbr/sweep_testing/actor_sweep.h>
#include <pbr/static_mesh_component.h>
#include <shape_box.h>


namespace pbr::sweep_testing {

constexpr static size_t VISIBILITY_MS = 333U;

void ActorSweep::FreeTransferResources ( VkDevice device ) noexcept
{
    _mesh->FreeTransferResources ( device );
}

void ActorSweep::Destroy () noexcept
{
    _shape.reset ();
    _mesh.reset ();
}

[[maybe_unused]] android_vulkan::ShapeRef const& ActorSweep::GetShape () noexcept
{
    return _shape;
}

bool ActorSweep::Init ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    VkCommandBuffer const* commandBuffers,
    GXVec3 const &location,
    GXVec3 const &size
) noexcept
{
    _visibilityTimeout = 0.0F;
    _shape = std::make_shared<android_vulkan::ShapeBox> ( size );

    GXMat4 transform {};
    transform.Translation ( location );
    _shape->UpdateCacheData ( transform );

    bool success;

    _mesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/System/Default.mtl",
        commandBuffers
    );

    if ( success )
        return true;

    _mesh.reset ();
    return false;
}

void ActorSweep::SetOverlay ( Texture2DRef const &overlay ) noexcept
{
    // NOLINTNEXTLINE
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );

    mesh.SetEmission (
        GXColorRGB (
            static_cast<GXUByte> ( 29U ),
            static_cast<GXUByte> ( 46U ),
            static_cast<GXUByte> ( 0U ),
            static_cast<GXUByte> ( 255U )
        )
    );

    // NOLINTNEXTLINE
    auto& material = static_cast<OpaqueMaterial&> ( *mesh.GetMaterial () );
    material.SetEmission ( overlay );
}

void ActorSweep::Submit ( RenderSession &renderSession ) noexcept
{
    if ( !_mesh || !_visible )
        return;

    // NOLINTNEXTLINE
    auto const& shape = static_cast<android_vulkan::ShapeBox const&> ( *_shape );
    GXMat4 const& shapeTransform = shape.GetTransformWorld ();

    constexpr float rendererScale = 32.0F;
    GXVec3 scale {};
    scale.Multiply ( shape.GetSize (), rendererScale );

    GXMat4 transform {};

    auto& x = *reinterpret_cast<GXVec3*> ( &transform._m[ 0U ][ 0U ] );
    x.Multiply ( *reinterpret_cast<GXVec3 const*> ( &shapeTransform._m[ 0U ][ 0U ] ), scale._data[ 0U ] );

    auto& y = *reinterpret_cast<GXVec3*> ( &transform._m[ 1U ][ 0U ] );
    y.Multiply ( *reinterpret_cast<GXVec3 const*> ( &shapeTransform._m[ 1U ][ 0U ] ), scale._data[ 1U ] );

    auto& z = *reinterpret_cast<GXVec3*> ( &transform._m[ 2U ][ 0U ] );
    z.Multiply ( *reinterpret_cast<GXVec3 const*> ( &shapeTransform._m[ 2U ][ 0U ] ), scale._data[ 2U ] );

    auto& location = *reinterpret_cast<GXVec3*> ( &transform._m[ 3U ][ 0U ] );
    location.Multiply ( *reinterpret_cast<GXVec3 const*> ( &shapeTransform._m[ 3U ][ 0U ] ), rendererScale );

    transform._m[ 0U ][ 3U ] = transform._m[ 1U ][ 3U ] = transform._m[ 2U ][ 3U ] = 0.0F;
    transform._m[ 3U ][ 3U ] = 1.0F;

    // NOLINTNEXTLINE
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );
    mesh.SetTransform ( transform );
    mesh.Submit ( renderSession );
}

void ActorSweep::Update ( float deltaTime ) noexcept
{
    if ( !_mesh )
        return;

    constexpr float timeout = 1.0e-3F * static_cast<float> ( VISIBILITY_MS );
    _visibilityTimeout += deltaTime;

    if ( _visibilityTimeout < timeout )
        return;

    while ( _visibilityTimeout > timeout )
        _visibilityTimeout -= timeout;

    _visible = !_visible;
}

} // namespace pbr::sweep_testing
