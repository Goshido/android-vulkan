#ifndef ANDROID_VULKAN_GLOBAL_FORCE_H
#define ANDROID_VULKAN_GLOBAL_FORCE_H


#include "rigid_body.h"


namespace android_vulkan {

class GlobalForce
{
    public:
        GlobalForce ( GlobalForce const & ) = delete;
        GlobalForce& operator = ( GlobalForce const & ) = delete;

        GlobalForce ( GlobalForce && ) = delete;
        GlobalForce& operator = ( GlobalForce && ) = delete;

        virtual ~GlobalForce () = default;

        virtual void Apply ( RigidBodyRef &rigidBody ) const noexcept = 0;

    protected:
        GlobalForce () = default;
};

using GlobalForceRef = std::shared_ptr<GlobalForce>;

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GLOBAL_FORCE_H
