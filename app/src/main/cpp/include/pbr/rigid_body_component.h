#ifndef PBR_RIGID_BODY_COMPONENT_H
#define PBR_RIGID_BODY_COMPONENT_H


#include "component.h"
#include <rigid_body.h>


namespace pbr {

class RigidBodyComponent final : public Component
{
    private:
        Actor*                          _actor;
        android_vulkan::RigidBodyRef    _rigidBody;

        static int                      _registerRigidBodyComponentIndex;

    public:
        RigidBodyComponent () = delete;

        RigidBodyComponent ( RigidBodyComponent const & ) = delete;
        RigidBodyComponent& operator = ( RigidBodyComponent const & ) = delete;

        RigidBodyComponent ( RigidBodyComponent && ) = delete;
        RigidBodyComponent& operator = ( RigidBodyComponent && ) = delete;

        explicit RigidBodyComponent ( android_vulkan::ShapeRef &shape ) noexcept;
        explicit RigidBodyComponent ( android_vulkan::ShapeRef &shape, std::string &&name ) noexcept;

        ~RigidBodyComponent () override = default;

        [[nodiscard]] android_vulkan::RigidBodyRef& GetRigidBody () noexcept;
        [[nodiscard]] bool Register ( Actor &actor, android_vulkan::Physics &physics, lua_State &vm ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;

    private:
        void Setup ( android_vulkan::ShapeRef &shape ) noexcept;

        [[nodiscard]] static int OnAddForce ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnGetLocation ( lua_State* state );
        [[nodiscard]] static int OnGetVelocityLinear ( lua_State* state );
        [[nodiscard]] static int OnSetVelocityLinear ( lua_State* state );

        static void OnTransformUpdate ( android_vulkan::RigidBody::Context context,
            GXVec3 const &location,
            GXQuat const &rotation
        ) noexcept;
};

} // namespace pbr


#endif // PBR_RIGID_BODY_COMPONENT_H
