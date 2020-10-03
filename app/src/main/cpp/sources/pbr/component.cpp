#include <pbr/component.h>
#include <pbr/static_mesh_component.h>


namespace pbr {

ComponentRef Component::Create ( size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers
)
{
    if ( desc._classID == ClassID::Unknown )
        return {};

    if ( desc._classID == ClassID::StaticMesh )
    {
        dataRead = sizeof ( pbr::StaticMeshComponentDesc );
        return std::make_shared<StaticMeshComponent> ( commandBufferConsumed, desc, data, renderer, commandBuffers );
    }

    return {};
}

} // namespace pbr
