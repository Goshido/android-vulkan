#ifndef PBR_RIGID_BODY_COMPONENT_HPP
#define PBR_RIGID_BODY_COMPONENT_HPP


#include "component.hpp"
#include "rigid_body_component_desc.hpp"
#include <rigid_body.hpp>


namespace pbr {

class RigidBodyComponent final : public Component
{
    private:
        Actor*                                                          _actor;
        android_vulkan::RigidBodyRef                                    _rigidBody;

        static int                                                      _registerRigidBodyComponentIndex;
        static std::unordered_map<Component const*, ComponentRef>       _rigidBodies;

    public:
        RigidBodyComponent () = delete;

        RigidBodyComponent ( RigidBodyComponent const & ) = delete;
        RigidBodyComponent &operator = ( RigidBodyComponent const & ) = delete;

        RigidBodyComponent ( RigidBodyComponent && ) = delete;
        RigidBodyComponent &operator = ( RigidBodyComponent && ) = delete;

        explicit RigidBodyComponent ( size_t &dataRead,
            RigidBodyComponentDesc const &desc,
            uint8_t const* data
        ) noexcept;

        explicit RigidBodyComponent ( std::string &&name ) noexcept;
        explicit RigidBodyComponent ( android_vulkan::ShapeRef &shape ) noexcept;
        explicit RigidBodyComponent ( android_vulkan::ShapeRef &shape, std::string &&name ) noexcept;

        ~RigidBodyComponent () override = default;

        [[nodiscard]] bool RegisterFromNative ( Actor &actor,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        [[nodiscard]] bool RegisterFromScript ( Actor &actor, android_vulkan::Physics &physics ) noexcept;
        void Unregister ( android_vulkan::Physics &physics ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] ComponentRef &GetReference () noexcept override;

        void Setup ( android_vulkan::ShapeRef &shape, bool forceAwake ) noexcept;

        [[nodiscard]] static int OnAddForce ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnGetLocation ( lua_State* state );
        [[nodiscard]] static int OnSetLocation ( lua_State* state );
        [[nodiscard]] static int OnSetShapeBox ( lua_State* state );
        [[nodiscard]] static int OnSetShapeSphere ( lua_State* state );
        [[nodiscard]] static int OnGetTransform ( lua_State* state );
        [[nodiscard]] static int OnGetVelocityLinear ( lua_State* state );
        [[nodiscard]] static int OnSetVelocityLinear ( lua_State* state );

        static void OnTransformUpdate ( android_vulkan::RigidBody::Context context,
            GXVec3 const &location,
            GXQuat const &rotation
        ) noexcept;
};

} // namespace pbr


#endif // PBR_RIGID_BODY_COMPONENT_HPP
