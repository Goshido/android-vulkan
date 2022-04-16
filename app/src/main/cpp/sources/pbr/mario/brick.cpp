#include <pbr/mario/brick.h>
#include <pbr/actor.h>
#include <pbr/rigid_body_component.h>
#include <pbr/static_mesh_component.h>
#include <guid_generator.h>
#include <shape_box.h>


namespace pbr::mario {

constexpr static GXVec3 COLLIDER_OFFSET ( -0.4F, 0.4F, 0.4F );
constexpr static GXVec3 COLLIDER_SIZE ( 0.8F, 0.8F, 0.8F );
constexpr static char const MATERIAL[] = "pbr/assets/Props/experimental/world-1-1/brick/brick.mtl";
constexpr static char const MESH[] = "pbr/assets/Props/experimental/world-1-1/brick/brick.mesh2";

//----------------------------------------------------------------------------------------------------------------------

// NOLINTNEXTLINE - method can be made static.
void Brick::Spawn ( android_vulkan::Renderer &renderer,
    VkCommandBuffer const*& commandBuffers,
    Scene &scene,
    float x,
    float y,
    float z
) noexcept
{
    bool success;
    size_t consumed;

    ComponentRef staticMesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        consumed,
        MESH,
        MATERIAL,
        commandBuffers,
        "Mesh"
    );

    commandBuffers += consumed;

    if ( !success )
        return;

    // NOLINTNEXTLINE - downcast.
    auto& mesh = static_cast<StaticMeshComponent&> ( *staticMesh );

    GXMat4 transform {};
    transform.Translation ( x, y, z );
    mesh.SetTransform ( transform );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( COLLIDER_SIZE._data[ 0U ],
        COLLIDER_SIZE._data[ 1U ],
        COLLIDER_SIZE._data[ 2U ]
    );

    ComponentRef rigidBody = std::make_shared<RigidBodyComponent> ( shape, "Collider" );

    // NOLINTNEXTLINE - downcast.
    auto& collider = static_cast<RigidBodyComponent&> ( *rigidBody );

    android_vulkan::RigidBody& body = *collider.GetRigidBody ();
    body.EnableKinematic ();

    constexpr float rendererToPhysics = 1.0F / 32.0F;
    GXVec3 origin {};
    origin.Multiply ( GXVec3 ( x, y, z ), rendererToPhysics );

    GXVec3 location {};
    location.Sum ( origin, COLLIDER_OFFSET );
    body.SetLocation ( location, false );

    ActorRef actor = std::make_shared<Actor> ( android_vulkan::GUID::GenerateAsString ( "Brick" ) );
    actor->AppendComponent ( staticMesh );
    actor->AppendComponent ( rigidBody );
    scene.AppendActor ( actor );
}

} // namespace pbr::mario
