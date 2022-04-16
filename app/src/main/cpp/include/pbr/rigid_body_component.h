#ifndef PBR_RIGID_BODY_COMPONENT_H
#define PBR_RIGID_BODY_COMPONENT_H


#include "component.h"
#include <rigid_body.h>


namespace pbr {

class RigidBodyComponent final : public Component
{
    private:
        android_vulkan::RigidBodyRef    _rigidBody;

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
        [[nodiscard]] bool Register ( android_vulkan::Physics &physics ) noexcept;
};

} // namespace pbr


#endif // PBR_RIGID_BODY_COMPONENT_H
