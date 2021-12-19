#include <pbr/mario/pipe_base.h>
#include <pbr/static_mesh_component.h>
#include <shape_box.h>


namespace pbr::mario {

constexpr static char const MATERIAL[] = "pbr/assets/Props/experimental/world-1-1/pipe/pipe.mtl";

android_vulkan::RigidBodyRef& PipeBase::GetCollider () noexcept
{
    return _collider;
}

ComponentRef& PipeBase::GetComponent () noexcept
{
    return _staticMesh;
}

void PipeBase::Init ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    VkCommandBuffer const* commandBuffers,
    float x,
    float y,
    float z
) noexcept
{
    bool success;

    _staticMesh = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        GetMesh (),
        MATERIAL,
        commandBuffers
    );

    if ( !success )
        return;

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
    location.Sum ( origin, GetColliderOffset () );
    body.SetLocation ( location, false );

    GXVec3 const& size = GetColliderSize ();

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( size._data[ 0U ],
        size._data[ 1U ],
        size._data[ 2U ]
    );

    body.SetShape ( shape, false );
}

PipeBase::PipeBase () noexcept:
    _collider {},
    _staticMesh {}
{
    // NOTHING
}

} // namespace pbr::mario
