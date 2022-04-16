#include <pbr/mario/obstacle.h>
#include <pbr/rigid_body_component.h>
#include <shape_box.h>
#include <guid_generator.h>


namespace pbr {

void Obstacle::Spawn ( Items const &items, Scene &scene, std::string &&name ) noexcept
{
    ActorRef actor = std::make_shared<Actor> ( std::move ( name ) );
    Actor& a = *actor;

    for ( auto const& item : items )
    {
        GXVec3 const& size = item._size;

        android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( size._data[ 0U ],
            size._data[ 1U ],
            size._data[ 2U ]
        );

        shape->SetFriction ( item._friction );

        ComponentRef rigidBody = std::make_shared<RigidBodyComponent> ( shape,
            android_vulkan::GUID::GenerateAsString ( "Collider" )
        );

        // NOLINTNEXTLINE - downcast.
        auto& component = static_cast<RigidBodyComponent&> ( *rigidBody );

        android_vulkan::RigidBody& collider = *component.GetRigidBody ();
        collider.EnableKinematic ();
        collider.SetLocation ( item._location, false );

        a.AppendComponent ( rigidBody );
    }

    scene.AppendActor ( actor );
}

} // namespace pbr
