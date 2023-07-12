#ifndef ANDROID_VULKAN_GLOBAL_FORCE_GRAVITY_HPP
#define ANDROID_VULKAN_GLOBAL_FORCE_GRAVITY_HPP


#include "global_force.hpp"


namespace android_vulkan {

class [[maybe_unused]] GlobalForceGravity final : public GlobalForce
{
    private:
        GXVec3 const    _freeFallAcceleration;

    public:
        GlobalForceGravity () = delete;

        GlobalForceGravity ( GlobalForceGravity const & ) = delete;
        GlobalForceGravity &operator = ( GlobalForceGravity const & ) = delete;

        GlobalForceGravity ( GlobalForceGravity && ) = delete;
        GlobalForceGravity &operator = ( GlobalForceGravity && ) = delete;

        explicit GlobalForceGravity ( GXVec3 const &freeFallAcceleration ) noexcept;

        ~GlobalForceGravity () override = default;

        void Apply ( RigidBodyRef const &rigidBody ) const noexcept override;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GLOBAL_FORCE_GRAVITY_HPP
