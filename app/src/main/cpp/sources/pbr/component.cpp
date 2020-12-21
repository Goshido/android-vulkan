#include <pbr/component.h>
#include <pbr/point_light_component.h>
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

    if ( desc._classID == ClassID::PointLight )
    {
        commandBufferConsumed = 0U;
        dataRead = sizeof ( PointLightComponentDesc );

        // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
        auto const& d = static_cast<PointLightComponentDesc const&> ( desc ); // NOLINT

        return std::make_shared<PointLightComponent> ( d );
    }

    if ( desc._classID == ClassID::StaticMesh )
    {
        dataRead = sizeof ( StaticMeshComponentDesc );

        // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
        auto const& d = static_cast<StaticMeshComponentDesc const&> ( desc ); // NOLINT

        return std::make_shared<StaticMeshComponent> ( commandBufferConsumed, d, data, renderer, commandBuffers );
    }

    return {};
}

} // namespace pbr
