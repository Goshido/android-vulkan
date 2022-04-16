#include <pbr/mario/pipe_base.h>
#include <pbr/rigid_body_component.h>
#include <pbr/static_mesh_component.h>
#include <guid_generator.h>
#include <shape_box.h>


namespace pbr::mario {

constexpr static char const MATERIAL[] = "pbr/assets/Props/experimental/world-1-1/pipe/pipe.mtl";

//----------------------------------------------------------------------------------------------------------------------

void PipeBase::SpawnBase ( android_vulkan::Renderer &renderer,
    VkCommandBuffer const*& commandBuffers,
    Scene &scene,
    float x,
    float y,
    float z,
    GXVec3 const &colliderOffset,
    GXVec3 const &colliderSize,
    char const* mesh
) noexcept
{
    bool success;
    size_t consumed;

    ComponentRef staticMesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        consumed,
        mesh,
        MATERIAL,
        commandBuffers,
        "Mesh"
    );

    commandBuffers += consumed;

    if ( !success )
        return;

    // NOLINTNEXTLINE - downcast.
    auto& m = static_cast<StaticMeshComponent&> ( *staticMesh );

    GXMat4 transform {};
    transform.Translation ( x, y, z );
    m.SetTransform ( transform );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( colliderSize._data[ 0U ],
        colliderSize._data[ 1U ],
        colliderSize._data[ 2U ]
    );

    ComponentRef rigidBody = std::make_shared<RigidBodyComponent> ( shape, "Collider" );

    // NOLINTNEXTLINE - downcast.
    auto& collider = static_cast<RigidBodyComponent&> ( *rigidBody );

    android_vulkan::RigidBody& body = *collider.GetRigidBody ();
    body.EnableKinematic ();

    constexpr float const rendererToPhysics = 1.0F / 32.0F;
    GXVec3 origin {};
    origin.Multiply ( GXVec3 ( x, y, z ), rendererToPhysics );

    GXVec3 location {};
    location.Sum ( origin, colliderOffset );
    body.SetLocation ( location, false );

    ActorRef actor = std::make_shared<Actor> ( android_vulkan::GUID::GenerateAsString ( "Pipe" ) );
    actor->AppendComponent ( staticMesh );
    actor->AppendComponent ( rigidBody );
    scene.AppendActor ( actor );
}

} // namespace pbr::mario
