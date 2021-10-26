#include <global_force_gravity.h>


namespace android_vulkan {

GlobalForceGravity::GlobalForceGravity ( GXVec3 const &freeFallAcceleration ) noexcept:
    _freeFallAcceleration ( freeFallAcceleration )
{
    // NOTHING
}

void GlobalForceGravity::Apply ( RigidBodyRef const &rigidBody ) const noexcept
{
    auto& body = *rigidBody;

    GXVec3 force {};
    force.Multiply ( _freeFallAcceleration, body.GetMass () );

    body.AddForce ( force, body.GetLocation (), false );
}

} // namespace android_vulkan
