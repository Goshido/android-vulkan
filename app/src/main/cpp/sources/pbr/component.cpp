#include <pbr/component.h>
#include <pbr/point_light_component.h>
//#include <pbr/point_light_component_desc.h>
#include <pbr/reflection_component.h>
#include <pbr/static_mesh_component.h>


namespace pbr {

ClassID Component::GetClassID () const
{
    return _classID;
}

ComponentRef Component::Create ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    if ( desc._classID == ClassID::Unknown )
    {
        commandBufferConsumed = 0U;
        dataRead = sizeof ( ComponentDesc );
        return {};
    }

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

        return std::make_shared<StaticMeshComponent> ( renderer, commandBufferConsumed, d, data, commandBuffers );
    }

    if ( desc._classID == ClassID::Reflection )
    {
        dataRead = sizeof ( ReflectionComponentDesc );

        // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
        auto const& d = static_cast<ReflectionComponentDesc const&> ( desc ); // NOLINT

        return std::make_shared<ReflectionComponent> ( renderer, commandBufferConsumed, d, data, commandBuffers );
    }

    commandBufferConsumed = 0U;
    dataRead = sizeof ( ComponentDesc );
    return {};
}

Component::Component ( ClassID classID ) noexcept:
    _classID ( classID )
{
    // NOTHING
}

} // namespace pbr
