#include <pbr/sweep_testing/actor_sweep.h>
#include <pbr/static_mesh_component.h>
#include <pbr/stipple_material.h>
#include <gamepad.h>
#include <shape_box.h>


namespace pbr::sweep_testing {

constexpr static float MOVING_SPEED = 1.25F;
constexpr static float STICK_DEAD_ZONE = 0.2F;

void ActorSweep::CaptureInput ( GXMat4 const &cameraLocal ) noexcept
{
    _cameraLeft = *reinterpret_cast<GXVec3 const*> ( &cameraLocal._m[ 0U ][ 0U ] );

    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.BindLeftStick ( this, &ActorSweep::OnLeftStick );
    gamepad.BindRightStick ( this, &ActorSweep::OnRightStick );
}

void ActorSweep::ReleaseInput () noexcept
{
    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.UnbindLeftStick ();
    gamepad.UnbindRightStick ();

    constexpr GXVec3 noMove ( 0.0F, 0.0F, 0.0F );
    _moveSpeed = noMove;
}

void ActorSweep::FreeTransferResources ( VkDevice device ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );
    mesh.FreeTransferResources ( device );
}

void ActorSweep::Destroy () noexcept
{
    _shape = nullptr;
    _mesh = nullptr;
}

android_vulkan::ShapeRef const& ActorSweep::GetShape () noexcept
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
    _shape = std::make_shared<android_vulkan::ShapeBox> ( size );

    GXMat4 transform {};
    transform.Translation ( location );
    _shape->UpdateCacheData ( transform );

    MaterialRef material = std::make_shared<StippleMaterial> ();

    bool success;

    _mesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        "pbr/system/unit-cube.mesh2",
        material,
        commandBuffers
    );

    if ( !success )
        return false;

    // NOLINTNEXTLINE - downcast.
    auto& m = static_cast<StaticMeshComponent&> ( *_mesh );
    m.SetColor0 ( GXColorRGB ( 1.0F, 1.0F, 1.0F, 0.5F ) );
    return true;
}

void ActorSweep::SetOverlay ( Texture2DRef const &overlay ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );

    mesh.SetEmission (
        GXColorRGB (
            static_cast<GXUByte> ( 29U ),
            static_cast<GXUByte> ( 46U ),
            static_cast<GXUByte> ( 0U ),
            static_cast<GXUByte> ( 255U )
        )
    );

    // NOLINTNEXTLINE - downcast.
    auto& material = static_cast<GeometryPassMaterial&> ( *mesh.GetMaterial () );
    material.SetEmission ( overlay );
}

void ActorSweep::Submit ( RenderSession &renderSession ) noexcept
{
    if ( !_mesh )
        return;

    // NOLINTNEXTLINE - downcast.
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

    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *_mesh );
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

    auto deadZoneHandler = [] ( float value ) -> float {
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
    auto& x = *reinterpret_cast<GXVec3*> ( &basis._m[ 0U ][ 0U ] );
    auto& y = *reinterpret_cast<GXVec3*> ( &basis._m[ 1U ][ 0U ] );
    auto& z = *reinterpret_cast<GXVec3*> ( &basis._m[ 2U ][ 0U ] );

    x = _cameraLeft;
    y = up;
    z.CrossProduct ( x, y );

    GXVec3 offset {};
    basis.MultiplyVectorMatrix ( offset, move );

    // NOLINTNEXTLINE - downcast.
    auto& shape = static_cast<android_vulkan::ShapeBox&> ( *_shape );
    GXMat4 transform = shape.GetTransformWorld ();
    auto& location = *reinterpret_cast<GXVec3*> ( &transform._m[ 3U ][ 0U ] );
    location.Sum ( location, MOVING_SPEED * deltaTime, offset );
    shape.UpdateCacheData ( transform );
}

void ActorSweep::OnLeftStick ( void* context, float horizontal, float vertical ) noexcept
{
    auto& actor = *static_cast<ActorSweep*> ( context );
    GXVec3& moveSpeed = actor._moveSpeed;
    moveSpeed._data[ 0U ] = horizontal;
    moveSpeed._data[ 2U ] = vertical;
}

void ActorSweep::OnRightStick ( void* context, float /*horizontal*/, float vertical ) noexcept
{
    auto& actor = *static_cast<ActorSweep*> ( context );
    actor._moveSpeed._data[ 1U ] = vertical;
}

} // namespace pbr::sweep_testing
