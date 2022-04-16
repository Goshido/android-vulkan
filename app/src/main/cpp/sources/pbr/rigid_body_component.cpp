#include <pbr/rigid_body_component.h>
#include <guid_generator.h>
#include <physics.h>


namespace pbr {

RigidBodyComponent::RigidBodyComponent ( android_vulkan::ShapeRef &shape ) noexcept:
    Component ( ClassID::RigidBody, android_vulkan::GUID::GenerateAsString ( "RigidBody" ) ),
    _rigidBody ( std::make_shared<android_vulkan::RigidBody> () )
{
    _rigidBody->SetShape ( shape, true );
}

RigidBodyComponent::RigidBodyComponent ( android_vulkan::ShapeRef &shape, std::string &&name  ) noexcept:
    Component ( ClassID::RigidBody, std::move ( name ) ),
    _rigidBody ( std::make_shared<android_vulkan::RigidBody> () )
{
    _rigidBody->SetShape ( shape, true );
}

android_vulkan::RigidBodyRef& RigidBodyComponent::GetRigidBody () noexcept
{
    return _rigidBody;
}

bool RigidBodyComponent::Register ( android_vulkan::Physics &physics ) noexcept
{
    return physics.AddRigidBody ( _rigidBody );
}

} // namespace pbr
