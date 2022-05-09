#include <pbr/rigid_body_component.h>
#include <pbr/actor.h>
#include <pbr/coordinate_system.h>
#include <guid_generator.h>
#include <physics.h>


namespace pbr {

RigidBodyComponent::RigidBodyComponent ( android_vulkan::ShapeRef &shape ) noexcept:
    Component ( ClassID::RigidBody, android_vulkan::GUID::GenerateAsString ( "RigidBody" ) ),
    _actor ( nullptr ),
    _rigidBody ( std::make_shared<android_vulkan::RigidBody> () )
{
    Init ( shape );
}

RigidBodyComponent::RigidBodyComponent ( android_vulkan::ShapeRef &shape, std::string &&name ) noexcept:
    Component ( ClassID::RigidBody, std::move ( name ) ),
    _actor ( nullptr ),
    _rigidBody ( std::make_shared<android_vulkan::RigidBody> () )
{
    Init ( shape );
}

android_vulkan::RigidBodyRef& RigidBodyComponent::GetRigidBody () noexcept
{
    return _rigidBody;
}

bool RigidBodyComponent::Register ( Actor &actor, android_vulkan::Physics &physics ) noexcept
{
    _actor = &actor;
    return physics.AddRigidBody ( _rigidBody );
}

void RigidBodyComponent::Init ( android_vulkan::ShapeRef &shape ) noexcept
{
    android_vulkan::RigidBody& body = *_rigidBody;
    body.SetShape ( shape, true );
    body.SetContext ( this );
    body.SetTransformUpdateHandler ( &RigidBodyComponent::OnTransformUpdate );
}

void RigidBodyComponent::OnTransformUpdate ( android_vulkan::RigidBody::Context context,
    GXVec3 const &location,
    GXQuat const &rotation
) noexcept
{
    auto& component = *static_cast<RigidBodyComponent*> ( context );

    GXVec3 origin {};
    origin.Multiply ( location, UNITS_IN_METER );

    GXMat4 transform {};
    transform.FromFast ( rotation, origin );

    component._actor->OnTransform ( transform );
}

} // namespace pbr
