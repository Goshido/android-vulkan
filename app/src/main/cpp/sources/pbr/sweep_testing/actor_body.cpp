#include <precompiled_headers.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/sweep_testing/actor_body.hpp>
#include <platform/android/pbr/opaque_material.hpp>
#include <platform/android/pbr/static_mesh_component.hpp>
#include <shape_box.hpp>


namespace pbr::sweep_testing {

constexpr float REROLL_PERIOD_SECONDS = 2.0F;
constexpr float ROTATION_SPEED = 5.0e-2F * GX_MATH_DOUBLE_PI;

//----------------------------------------------------------------------------------------------------------------------

GXColorUNORM const ActorBody::_overlayColor ( 101U, 49U, 15U, 255U );

void ActorBody::EnableOverlay () noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &mesh = static_cast<StaticMeshComponent &> ( *_mesh );
    mesh.SetEmission ( _overlayColor, 1.0F );
}

void ActorBody::DisableOverlay () noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &mesh = static_cast<StaticMeshComponent &> ( *_mesh );

    constexpr GXColorUNORM noOverlay ( 0U, 0U, 0U, 255U );
    mesh.SetEmission ( noOverlay, 1.0F );
}

void ActorBody::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &mesh = static_cast<StaticMeshComponent &> ( *_mesh );
    mesh.FreeTransferResources ( renderer );
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
    VkFence const* fences,
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
        fences,
        "Mesh"
    );

    if ( !success )
    {
        _mesh.reset ();
        return false;
    }

    _body = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody &ph = *_body;
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
    auto &mesh = static_cast<StaticMeshComponent &> ( *_mesh );

    // NOLINTNEXTLINE - downcast.
    auto &material = static_cast<OpaqueMaterial &> ( *mesh.GetMaterial () );
    material.SetEmission ( overlay );

    DisableOverlay ();
}

void ActorBody::Submit ( RenderSession &renderSession ) noexcept
{
    if ( !_mesh )
        return;

    android_vulkan::RigidBody &ph = *_body;

    GXMat3 orientation {};
    orientation.FromFast ( ph.GetRotation () );

    // NOLINTNEXTLINE
    auto &shape = static_cast<android_vulkan::ShapeBox &> ( ph.GetShape () );

    GXVec3 scale {};
    scale.Multiply ( shape.GetSize (), UNITS_IN_METER );

    GXMat4 transform {};
    auto &m = transform._data;

    auto &x = *reinterpret_cast<GXVec3*> ( m[ 0U ] );
    x.Multiply ( *reinterpret_cast<GXVec3 const*> ( orientation._data[ 0U ] ), scale._data[ 0U ] );

    auto &y = *reinterpret_cast<GXVec3*> ( m[ 1U ] );
    y.Multiply ( *reinterpret_cast<GXVec3 const*> ( orientation._data[ 1U ] ), scale._data[ 1U ] );

    auto &z = *reinterpret_cast<GXVec3*> ( m[ 2U ] );
    z.Multiply ( *reinterpret_cast<GXVec3 const*> ( orientation._data[ 2U ] ), scale._data[ 2U ] );

    auto &location = *reinterpret_cast<GXVec3*> ( m[ 3U ] );
    location.Multiply ( ph.GetLocation (), UNITS_IN_METER );

    m[ 0U ][ 3U ] = 0.0F;
    m[ 1U ][ 3U ] = 0.0F;
    m[ 2U ][ 3U ] = 0.0F;
    m[ 3U ][ 3U ] = 1.0F;

    // NOLINTNEXTLINE - downcast.
    auto &mesh = *static_cast<StaticMeshComponent*> ( _mesh.get () );
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
