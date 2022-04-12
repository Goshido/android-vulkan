#include <pbr/sweep_testing/actor_body.h>
#include <pbr/opaque_material.h>
#include <pbr/static_mesh_component.h>
#include <shape_box.h>


namespace pbr::sweep_testing {

constexpr float REROLL_PERIOD_SECONDS = 2.0F;
constexpr float ROTATION_SPEED = 5.0e-2F * GX_MATH_DOUBLE_PI;

//----------------------------------------------------------------------------------------------------------------------

GXColorRGB const ActorBody::_overlayColor (
    static_cast<GXUByte> ( 101U ),
    static_cast<GXUByte> ( 49U ),
    static_cast<GXUByte> ( 15U ),
    static_cast<GXUByte> ( 255U )
);

void ActorBody::EnableOverlay () noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );
    mesh.SetEmission ( _overlayColor );
}

void ActorBody::DisableOverlay () noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );

    constexpr GXColorRGB noOverlay ( 0.0F, 0.0F, 0.0F, 1.0F );
    mesh.SetEmission ( noOverlay );
}

void ActorBody::FreeTransferResources ( VkDevice device ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );
    mesh.FreeTransferResources ( device );
}

void ActorBody::Destroy () noexcept
{
    _body.reset ();
    _mesh.reset ();
}

bool ActorBody::Init ( android_vulkan::Renderer &renderer,
    android_vulkan::Physics &physics,
    size_t &commandBufferConsumed,
    VkCommandBuffer const* commandBuffers,
    GXVec3 const &location,
    GXVec3 const &size
) noexcept
{
    GXRandomize ();
    _angular0 = GenerateAngular ();
    _angular1 = GenerateAngular ();
    _angularSlider = 0.0F;

    bool success;

    _mesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/System/DefaultCSG.mtl",
        commandBuffers,
        "Mesh"
    );

    if ( !success )
    {
        _mesh.reset ();
        return false;
    }

    _body = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& ph = *_body;
    ph.EnableKinematic ();
    ph.SetLocation ( location, false );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( size );

    constexpr uint32_t groups = 0b00000000'00000000'00000000'00000010U;
    shape->SetCollisionGroups ( groups );

    ph.SetShape ( shape, false );
    ph.SetContext ( this );

    return physics.AddRigidBody ( _body );
}

void ActorBody::SetOverlay ( Texture2DRef const &overlay ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );

    // NOLINTNEXTLINE - downcast.
    auto& material = static_cast<OpaqueMaterial&> ( *mesh.GetMaterial () );
    material.SetEmission ( overlay );

    DisableOverlay ();
}

void ActorBody::Submit ( RenderSession &renderSession ) noexcept
{
    if ( !_mesh )
        return;

    android_vulkan::RigidBody& ph = *_body;

    GXMat3 orientation {};
    orientation.FromFast ( ph.GetRotation () );

    // NOLINTNEXTLINE
    auto& shape = static_cast<android_vulkan::ShapeBox&> ( ph.GetShape () );

    constexpr float rendererScale = 32.0F;
    GXVec3 scale {};
    scale.Multiply ( shape.GetSize (), rendererScale );

    GXMat4 transform {};

    auto& x = *reinterpret_cast<GXVec3*> ( &transform._m[ 0U ][ 0U ] );
    x.Multiply ( *reinterpret_cast<GXVec3 const*> ( &orientation._m[ 0U ][ 0U ] ), scale._data[ 0U ] );

    auto& y = *reinterpret_cast<GXVec3*> ( &transform._m[ 1U ][ 0U ] );
    y.Multiply ( *reinterpret_cast<GXVec3 const*> ( &orientation._m[ 1U ][ 0U ] ), scale._data[ 1U ] );

    auto& z = *reinterpret_cast<GXVec3*> ( &transform._m[ 2U ][ 0U ] );
    z.Multiply ( *reinterpret_cast<GXVec3 const*> ( &orientation._m[ 2U ][ 0U ] ), scale._data[ 2U ] );

    auto& location = *reinterpret_cast<GXVec3*> ( &transform._m[ 3U ][ 0U ] );
    location.Multiply ( ph.GetLocation (), rendererScale );

    transform._m[ 0U ][ 3U ] = transform._m[ 1U ][ 3U ] = transform._m[ 2U ][ 3U ] = 0.0F;
    transform._m[ 3U ][ 3U ] = 1.0F;

    // NOLINTNEXTLINE - downcast.
    auto& mesh = *static_cast<StaticMeshComponent*> ( _mesh.get () );
    mesh.SetTransform ( transform );
    mesh.Submit ( renderSession );
}

void ActorBody::Update ( float deltaTime ) noexcept
{
    constexpr float timeSpeed = 1.0F / REROLL_PERIOD_SECONDS;
    _angularSlider += deltaTime * timeSpeed;

    if ( _angularSlider > 1.0F )
    {
        while ( _angularSlider > 1.0F )
            _angularSlider -= 1.0F;

        _angular0 = _angular1;
        _angular1 = GenerateAngular ();
    }

    GXVec3 a {};
    a.LinearInterpolation ( _angular0, _angular1, _angularSlider );
    _body->SetVelocityAngular ( a, false );
}

GXVec3 ActorBody::GenerateAngular () noexcept
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

} // namespace pbr::sweep_testing
