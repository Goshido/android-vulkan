#include <pbr/mario/brick.h>
#include <pbr/static_mesh_component.h>
#include <shape_box.h>


namespace pbr::mario {

constexpr static GXVec3 const COLLIDER_OFFSET ( -0.4F, 0.4F, 0.4F );
constexpr static GXVec3 const COLLIDER_SIZE ( 0.8F, 0.8F, 0.8F );
constexpr static char const MATERIAL[] = "pbr/assets/Props/experimental/world-1-1/brick/brick.mtl";
constexpr static char const MESH[] = "pbr/assets/Props/experimental/world-1-1/brick/brick.mesh2";

Brick::Brick () noexcept:
    _collider {},
    _staticMesh {}
{
    // NOTHING
}

android_vulkan::RigidBodyRef& Brick::GetCollider () noexcept
{
    return _collider;
}

ComponentRef& Brick::GetComponent () noexcept
{
    return _staticMesh;
}

void Brick::Init ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    VkCommandBuffer const* commandBuffers,
    float x,
    float y,
    float z
) noexcept
{
    _staticMesh = std::make_shared<StaticMeshComponent> ( renderer,
        commandBufferConsumed,
        MESH,
        MATERIAL,
        commandBuffers
    );

    // NOLINTNEXTLINE
    auto& comp = *static_cast<StaticMeshComponent*> ( _staticMesh.get () );

    GXMat4 transform {};
    transform.Translation ( x, y, z );
    comp.SetTransform ( transform );

    _collider = std::make_shared<android_vulkan::RigidBody> ();
    android_vulkan::RigidBody& body = *_collider.get ();
    body.EnableKinematic ();

    constexpr float const rendererToPhysics = 1.0F / 32.0F;
    GXVec3 origin {};
    origin.Multiply ( GXVec3 ( x, y, z ), rendererToPhysics );

    GXVec3 location {};
    location.Sum ( origin, COLLIDER_OFFSET );
    body.SetLocation ( location );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( COLLIDER_SIZE._data[ 0U ],
        COLLIDER_SIZE._data[ 1U ],
        COLLIDER_SIZE._data[ 2U ]
    );

    body.SetShape ( shape );
}

} // namespace pbr::mario