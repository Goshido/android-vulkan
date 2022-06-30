#include <pbr/mario/pipe_base.h>
#include <pbr/coordinate_system.h>
#include <pbr/rigid_body_component.h>
#include <guid_generator.h>
#include <shape_box.h>


namespace pbr::mario {

void PipeBase::SpawnBase ( Scene &scene,
    float x,
    float y,
    float z,
    GXVec3 const &colliderOffset,
    GXVec3 const &colliderSize
) noexcept
{
    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( colliderSize._data[ 0U ],
        colliderSize._data[ 1U ],
        colliderSize._data[ 2U ]
    );

    ComponentRef rigidBody = std::make_shared<RigidBodyComponent> ( shape, "Collider" );

    // NOLINTNEXTLINE - downcast.
    auto& collider = static_cast<RigidBodyComponent&> ( *rigidBody );

    android_vulkan::RigidBody& body = *collider.GetRigidBody ();
    body.EnableKinematic ();

    GXVec3 origin {};
    origin.Multiply ( GXVec3 ( x, y, z ), METERS_IN_UNIT );

    GXVec3 location {};
    location.Sum ( origin, colliderOffset );
    body.SetLocation ( location, false );

    ActorRef actor = std::make_shared<Actor> ( android_vulkan::GUID::GenerateAsString ( "Pipe" ) );
    actor->AppendComponent ( rigidBody );
    scene.AppendActor ( actor );
}

} // namespace pbr::mario
