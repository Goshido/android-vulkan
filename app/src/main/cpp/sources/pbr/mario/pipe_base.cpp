#include <pbr/mario/pipe_base.h>
#include <pbr/static_mesh_component.h>
#include <guid_generator.h>
#include <shape_box.h>


namespace pbr::mario {

constexpr static char const MATERIAL[] = "pbr/assets/Props/experimental/world-1-1/pipe/pipe.mtl";

//----------------------------------------------------------------------------------------------------------------------

void PipeBase::Init ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    VkCommandBuffer const* commandBuffers,
    Scene &scene,
    android_vulkan::Physics &physics,
    float x,
    float y,
    float z
) noexcept
{
    bool success;

    ComponentRef staticMesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        GetMesh (),
        MATERIAL,
        commandBuffers,
        "Mesh"
    );

    if ( !success )
        return;

    // NOLINTNEXTLINE
    auto& comp = static_cast<StaticMeshComponent&> ( *staticMesh );

    GXMat4 transform {};
    transform.Translation ( x, y, z );
    comp.SetTransform ( transform );

    android_vulkan::RigidBodyRef collider = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& body = *collider;
    body.EnableKinematic ();

    constexpr float const rendererToPhysics = 1.0F / 32.0F;
    GXVec3 origin {};
    origin.Multiply ( GXVec3 ( x, y, z ), rendererToPhysics );

    GXVec3 location {};
    location.Sum ( origin, GetColliderOffset () );
    body.SetLocation ( location, false );

    GXVec3 const& size = GetColliderSize ();

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( size._data[ 0U ],
        size._data[ 1U ],
        size._data[ 2U ]
    );

    body.SetShape ( shape, false );

    if ( !physics.AddRigidBody ( collider ) )
        return;

    ActorRef actor = std::make_shared<Actor> ( android_vulkan::GUID::GenerateAsString ( "Pipe" ) );
    actor->AppendComponent ( staticMesh );
    scene.AppendActor ( actor );
}

} // namespace pbr::mario
