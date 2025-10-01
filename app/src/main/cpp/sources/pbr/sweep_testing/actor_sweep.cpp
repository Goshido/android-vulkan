#include <precompiled_headers.hpp>
#include <pbr/sweep_testing/actor_sweep.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/static_mesh_component.hpp>
#include <pbr/stipple_material.hpp>
#include <gamepad.hpp>
#include <shape_box.hpp>


namespace pbr::sweep_testing {

namespace {

constexpr float MOVING_SPEED = 1.25F;
constexpr float STICK_DEAD_ZONE = 0.2F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void ActorSweep::CaptureInput ( GXMat4 const &cameraLocal ) noexcept
{
    _cameraLeft = *reinterpret_cast<GXVec3 const*> ( cameraLocal._data[ 0U ] );

    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.BindLeftStick ( this, &ActorSweep::OnLeftStick );
    gamepad.BindRightStick ( this, &ActorSweep::OnRightStick );
}

void ActorSweep::ReleaseInput () noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.UnbindLeftStick ();
    gamepad.UnbindRightStick ();

    constexpr GXVec3 noMove ( 0.0F, 0.0F, 0.0F );
    _moveSpeed = noMove;
}

void ActorSweep::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &mesh = static_cast<StaticMeshComponent &> ( *_mesh );
    mesh.FreeTransferResources ( renderer );
}

void ActorSweep::Destroy () noexcept
{
    _shape = nullptr;
    _mesh = nullptr;
}

android_vulkan::ShapeRef const &ActorSweep::GetShape () noexcept
{
    return _shape;
}

bool ActorSweep::Init ( android_vulkan::Renderer &renderer,
    GXVec3 const &location,
    GXVec3 const &size
) noexcept
{
    _shape = std::make_shared<android_vulkan::ShapeBox> ( size );

    GXMat4 transform {};
    transform.Translation ( location );
    _shape->UpdateCacheData ( transform );

    MaterialRef material = std::make_shared<StippleMaterial> ();

    bool success;

    _mesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        "pbr/system/unit-cube.mesh2",
        material
    );

    if ( !success )
        return false;

    // NOLINTNEXTLINE - downcast.
    auto &m = static_cast<StaticMeshComponent &> ( *_mesh );
    m.SetColor0 ( GXColorUNORM ( 255U, 255U, 255U, 128U ) );
    return true;
}

void ActorSweep::SetOverlay ( Texture2DRef const &overlay ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &mesh = static_cast<StaticMeshComponent &> ( *_mesh );

    mesh.SetEmission ( GXColorUNORM ( 29U, 46U, 0U, 255U ), 1.0F );

    // NOLINTNEXTLINE - downcast.
    auto &material = static_cast<GeometryPassMaterial &> ( *mesh.GetMaterial () );
    material.SetEmission ( overlay );
}

void ActorSweep::Submit ( RenderSession &renderSession ) noexcept
{
    if ( !_mesh )
        return;

    // NOLINTNEXTLINE - downcast.
    auto const &shape = static_cast<android_vulkan::ShapeBox const &> ( *_shape );
    GXMat4 const &shapeTransform = shape.GetTransformWorld ();

    GXVec3 scale {};
    scale.Multiply ( shape.GetSize (), UNITS_IN_METER );

    GXMat4 transform {};
    auto &m = transform._data;

    auto &x = *reinterpret_cast<GXVec3*> ( m[ 0U ] );
    x.Multiply ( *reinterpret_cast<GXVec3 const*> ( shapeTransform._data[ 0U ] ), scale._data[ 0U ] );

    auto &y = *reinterpret_cast<GXVec3*> ( m[ 1U ] );
    y.Multiply ( *reinterpret_cast<GXVec3 const*> ( shapeTransform._data[ 1U ] ), scale._data[ 1U ] );

    auto &z = *reinterpret_cast<GXVec3*> ( m[ 2U ] );
    z.Multiply ( *reinterpret_cast<GXVec3 const*> ( shapeTransform._data[ 2U ] ), scale._data[ 2U ] );

    auto &location = *reinterpret_cast<GXVec3*> ( m[ 3U ] );
    location.Multiply ( *reinterpret_cast<GXVec3 const*> ( shapeTransform._data[ 3U ] ), UNITS_IN_METER );

    m[ 0U ][ 3U ] = 0.0F;
    m[ 1U ][ 3U ] = 0.0F;
    m[ 2U ][ 3U ] = 0.0F;
    m[ 3U ][ 3U ] = 1.0F;

    // NOLINTNEXTLINE - downcast.
    auto &mesh = static_cast<StaticMeshComponent &> ( *_mesh );
    mesh.SetTransform ( transform );
    mesh.Submit ( renderSession );
}

void ActorSweep::Update ( float deltaTime ) noexcept
{
    if ( !_mesh )
        return;

    UpdateLocation ( deltaTime );
}

void ActorSweep::UpdateLocation ( float deltaTime ) noexcept
{
    GXVec3 move = _moveSpeed;

    constexpr auto deadZoneHandler = [] ( float value ) -> float {
        if ( std::abs ( value ) < STICK_DEAD_ZONE )
            return 0.0F;

        constexpr float scale = 1.0F / ( 1.0F - STICK_DEAD_ZONE );
        float const threshold = value > 0.0F ? -STICK_DEAD_ZONE : STICK_DEAD_ZONE;
        return scale * ( value + threshold );
    };

    move._data[ 0U ] = deadZoneHandler ( move._data[ 0U ] );
    move._data[ 1U ] = deadZoneHandler ( move._data[ 1U ] );
    move._data[ 2U ] = deadZoneHandler ( move._data[ 2U ] );

    constexpr GXVec3 up ( 0.0F, 1.0F, 0.0F );

    GXMat3 basis {};
    auto &x = *reinterpret_cast<GXVec3*> ( basis._data[ 0U ] );
    auto &y = *reinterpret_cast<GXVec3*> ( basis._data[ 1U ] );
    auto &z = *reinterpret_cast<GXVec3*> ( basis._data[ 2U ] );

    x = _cameraLeft;
    y = up;
    z.CrossProduct ( x, y );

    GXVec3 offset {};
    basis.MultiplyVectorMatrix ( offset, move );

    // NOLINTNEXTLINE - downcast.
    auto &shape = static_cast<android_vulkan::ShapeBox &> ( *_shape );
    GXMat4 transform = shape.GetTransformWorld ();
    auto &location = *reinterpret_cast<GXVec3*> ( transform._data[ 3U ] );
    location.Sum ( location, MOVING_SPEED * deltaTime, offset );
    shape.UpdateCacheData ( transform );
}

void ActorSweep::OnLeftStick ( void* context, float horizontal, float vertical ) noexcept
{
    auto &actor = *static_cast<ActorSweep*> ( context );
    GXVec3 &moveSpeed = actor._moveSpeed;
    moveSpeed._data[ 0U ] = horizontal;
    moveSpeed._data[ 2U ] = vertical;
}

void ActorSweep::OnRightStick ( void* context, float /*horizontal*/, float vertical ) noexcept
{
    auto &actor = *static_cast<ActorSweep*> ( context );
    actor._moveSpeed._data[ 1U ] = vertical;
}

} // namespace pbr::sweep_testing
