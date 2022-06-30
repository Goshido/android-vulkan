#include <pbr/mario/riddle.h>
#include <pbr/coordinate_system.h>
#include <pbr/rigid_body_component.h>
#include <guid_generator.h>
#include <shape_box.h>


namespace pbr::mario {

constexpr static GXVec3 COLLIDER_OFFSET ( -0.4F, 0.4F, 0.4F );
constexpr static GXVec3 COLLIDER_SIZE ( 0.8F, 0.8F, 0.8F );

//----------------------------------------------------------------------------------------------------------------------

void Riddle::Spawn ( Scene &scene, float x, float y, float z ) noexcept
{
    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( COLLIDER_SIZE._data[ 0U ],
        COLLIDER_SIZE._data[ 1U ],
        COLLIDER_SIZE._data[ 2U ]
    );

    ComponentRef rigidBody = std::make_shared<RigidBodyComponent> ( shape, "Collider" );

    // NOLINTNEXTLINE - downcast.
    auto& collider = static_cast<RigidBodyComponent&> ( *rigidBody );

    android_vulkan::RigidBody& body = *collider.GetRigidBody ();
    body.EnableKinematic ();

    GXVec3 origin {};
    origin.Multiply ( GXVec3 ( x, y, z ), METERS_IN_UNIT );

    GXVec3 location {};
    location.Sum ( origin, COLLIDER_OFFSET );
    body.SetLocation ( location, false );

    ActorRef actor = std::make_shared<Actor> ( android_vulkan::GUID::GenerateAsString ( "Riddle" ) );
    actor->AppendComponent ( rigidBody );
    scene.AppendActor ( actor );
}

} // namespace pbr::mario
