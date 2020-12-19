#include <pbr/component.h>
#include <pbr/point_light_component_desc.h>
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

    if ( desc._classID == ClassID::PointLight )
    {
        commandBufferConsumed = 0U;
        dataRead = sizeof ( pbr::PointLightComponentDesc );
        // TODO
        return {};
    }

    return {};
}

} // namespace pbr
