#include <pbr/sweep_testing/actor.h>
#include <pbr/opaque_material.h>
#include <pbr/static_mesh_component.h>
#include <shape_box.h>


namespace pbr::sweep_testing {

[[maybe_unused]] void Actor::EnableOverlay ( Texture2DRef const &overlay ) noexcept
{
    // NOLINTNEXTLINE
    auto& mesh = *static_cast<StaticMeshComponent*> ( _mesh.get () );

    // NOLINTNEXTLINE
    auto& material = *static_cast<OpaqueMaterial*> ( mesh.GetMaterial ().get () );
    material.SetEmission ( overlay );
}

[[maybe_unused]] void Actor::DisableOverlay () noexcept
{
    // NOLINTNEXTLINE
    auto& mesh = *static_cast<StaticMeshComponent*> ( _mesh.get () );

    // NOLINTNEXTLINE
    auto& material = *static_cast<OpaqueMaterial*> ( mesh.GetMaterial ().get () );
    material.SetEmissionDefault ();
}

void Actor::FreeTransferResources ( VkDevice device ) noexcept
{
    _mesh->FreeTransferResources ( device );
}

void Actor::Destroy () noexcept
{
    _body.reset ();
    _mesh.reset ();
}

bool Actor::Init ( android_vulkan::Renderer &renderer,
    android_vulkan::Physics &physics,
    size_t &commandBufferConsumed,
    VkCommandBuffer const* commandBuffers,
    GXVec3 const &location,
    GXVec3 const &size
) noexcept
{
    _mesh = std::make_shared<StaticMeshComponent> ( renderer,
        commandBufferConsumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/System/DefaultCSG.mtl",
        commandBuffers
    );

    if ( !_mesh )
        return false;

    _body = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& ph = *_body.get ();
    ph.EnableKinematic ();
    ph.SetLocation ( location, false );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( size );
    ph.SetShape ( shape, false );

    return physics.AddRigidBody ( _body );
}

void Actor::Submit ( RenderSession &renderSession ) noexcept
{
    if ( !_mesh )
        return;

    android_vulkan::RigidBody& ph = *_body.get ();

    GXMat3 orientation {};
    orientation.FromFast ( ph.GetRotation () );

    // NOLINTNEXTLINE
    auto& shape = static_cast<android_vulkan::ShapeBox&> ( ph.GetShape () );

    constexpr float rendererScale = 32.0F;
    GXVec3 scale {};
    scale.Multiply ( shape.GetSize (), rendererScale );

    GXVec3 location {};
    location.Multiply ( ph.GetLocation (), rendererScale );

    GXVec3 x {};
    x.Multiply ( *reinterpret_cast<GXVec3 const*> ( &orientation._m[ 0U ][ 0U ] ), scale._data[ 0U ] );

    GXMat4 transform {};
    transform.SetW ( location );

    GXVec3 y {};
    y.Multiply ( *reinterpret_cast<GXVec3 const*> ( &orientation._m[ 1U ][ 0U ] ), scale._data[ 1U ] );

    transform.SetX ( x );

    GXVec3 z {};
    z.Multiply ( *reinterpret_cast<GXVec3 const*> ( &orientation._m[ 2U ][ 0U ] ), scale._data[ 2U ] );

    transform.SetY ( y );
    transform.SetZ ( z );

    transform._m[ 0U ][ 3U ] = transform._m[ 1U ][ 3U ] = transform._m[ 2U ][ 3U ] = 0.0F;
    transform._m[ 3U ][ 3U ] = 1.0F;

    // NOLINTNEXTLINE
    auto& mesh = *static_cast<StaticMeshComponent*> ( _mesh.get () );
    mesh.SetTransform ( transform );
    mesh.Submit ( renderSession );
}

void Actor::Update ( float /*deltaTime*/ ) noexcept
{
    // TODO
}

} // namespace pbr::sweep_testing
