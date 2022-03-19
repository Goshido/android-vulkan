#include <pbr/mario/mario.h>
#include <pbr/static_mesh_component.h>
#include <gamepad.h>
#include <shape_box.h>
//#include <shape_sphere.h>


namespace pbr::mario {

constexpr static GXVec3 COLLIDER_SIZE ( 0.8F, 0.8F, 0.8F );
constexpr static float DAMPING_ANGULAR = 0.99F;
constexpr static float DAMPING_LINEAR = 0.99F;
constexpr static float DEAD_ZONE = 0.2F;
constexpr static float FRICTION = 0.5F;
constexpr static float JUMP_SPEED = 9.0F;
constexpr static float MASS = 38.0F;
constexpr static char const MATERIAL[] = "pbr/assets/Props/experimental/world-1-1/mario/mario.mtl";
constexpr static char const MESH[] = "pbr/assets/Props/experimental/world-1-1/mario/mario-cube.mesh2";
constexpr static GXVec3 MOVE_FORCE ( 0.0F, 0.0F, 500.0F );
constexpr static float MOVE_SPEED = 5.0F;
constexpr static float RESTITUTION = 0.0F;

//----------------------------------------------------------------------------------------------------------------------

GXMat4 const& Mario::GetTransform () const noexcept
{
    // NOLINTNEXTLINE
    auto& comp = *static_cast<StaticMeshComponent*> ( _staticMesh.get () );
    return comp.GetTransform ();
}

void Mario::CaptureInput () noexcept
{
    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.BindLeftStick ( this, &Mario::OnLeftStick );
    gamepad.BindKey ( this, &Mario::OnADown, android_vulkan::eGamepadKey::A, android_vulkan::eButtonState::Down );
}

ComponentRef& Mario::GetComponent () noexcept
{
    return _staticMesh;
}

android_vulkan::RigidBodyRef& Mario::GetRigidBody () noexcept
{
    return _rigidBody;
}

void Mario::Init ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    VkCommandBuffer const* commandBuffers,
    float x,
    float y,
    float z
) noexcept
{
    bool success;

    _staticMesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        MESH,
        MATERIAL,
        commandBuffers
    );

    if ( !success )
        return;

    _rigidBody = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& body = *_rigidBody.get ();
    body.DisableKinematic ( true );
    body.EnableSleep ();
    body.SetDampingAngular ( DAMPING_ANGULAR );
    body.SetDampingLinear ( DAMPING_LINEAR );
    body.SetMass ( MASS, true );
    body.SetLocation ( x, y, z, true );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( COLLIDER_SIZE._data[ 0U ],
        COLLIDER_SIZE._data[ 1U ],
        COLLIDER_SIZE._data[ 2U ]
    );

    shape->SetFriction ( FRICTION );
    shape->SetRestitution ( RESTITUTION );

    body.SetShape ( shape, true );
    OnUpdate ();
}

void Mario::OnUpdate () noexcept
{
    auto& body = *_rigidBody.get ();
    GXMat4 transform = body.GetTransform ();
    GXVec3 bodyLoc {};
    transform.GetW ( bodyLoc );

    constexpr float const physicsToRender = 32.0F;
    GXVec3 location {};
    location.Multiply ( bodyLoc, physicsToRender );
    transform.SetW ( location );

    // NOLINTNEXTLINE
    auto& comp = *static_cast<StaticMeshComponent*> ( _staticMesh.get () );
    comp.SetTransform ( transform );

    GXVec3 velocityLinear = body.GetVelocityLinear ();

    if ( _isJump )
    {
        velocityLinear._data[ 1U ] += JUMP_SPEED;
        body.SetVelocityLinear ( velocityLinear, true );
        _isJump = false;
    }

    bool const isMove = ( ( _move < 0.0F ) & ( velocityLinear._data[ 2U ] > -MOVE_SPEED ) ) |
        ( ( _move > 0.0F ) & ( velocityLinear._data[ 2U ] < MOVE_SPEED ) );

    if ( !isMove )
        return;

    GXVec3 force {};
    force.Multiply ( MOVE_FORCE, _move );
    body.AddForce ( force, bodyLoc, true );
}

void Mario::ReleaseInput () noexcept
{
    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.UnbindLeftStick ();
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::A, android_vulkan::eButtonState::Down );
}

void Mario::OnADown ( void* context ) noexcept
{
    auto& mario = *static_cast<Mario*> ( context );
    mario._isJump = true;
}

void Mario::OnLeftStick ( void* context, float horizontal, float /*vertical*/ ) noexcept
{
    auto& mario = *static_cast<Mario*> ( context );

    if ( std::abs ( horizontal ) < DEAD_ZONE )
    {
        mario._move = 0.0F;
        return;
    }

    constexpr float const deadZoneFactor = 1.0F / ( 1.0F - DEAD_ZONE );

    mario._move = horizontal >= 0.0F ?
        ( horizontal - DEAD_ZONE ) * deadZoneFactor :
        ( horizontal + DEAD_ZONE ) * deadZoneFactor;
}

} // namespace pbr::mario
