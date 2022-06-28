#include <pbr/component.h>
#include <pbr/point_light_component.h>
#include <pbr/reflection_component.h>
#include <pbr/static_mesh_component.h>
#include <guid_generator.h>


namespace pbr {

ClassID Component::GetClassID () const noexcept
{
    return _classID;
}

std::string const& Component::GetName () const noexcept
{
    return _name;
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

        // NOLINTNEXTLINE - downcast.
        auto const& d = static_cast<PointLightComponentDesc const&> ( desc );

        return std::make_shared<PointLightComponent> ( d, data );
    }

    if ( desc._classID == ClassID::StaticMesh )
    {
        dataRead = sizeof ( StaticMeshComponentDesc );

        // NOLINTNEXTLINE - downcast.
        auto const& d = static_cast<StaticMeshComponentDesc const&> ( desc );

        bool success;

        ComponentRef result = std::make_shared<StaticMeshComponent> ( renderer,
            success,
            commandBufferConsumed,
            d,
            data,
            commandBuffers
        );

        return success ? result : ComponentRef {};
    }

    if ( desc._classID == ClassID::Reflection )
    {
        dataRead = sizeof ( ReflectionComponentDesc );

        // NOLINTNEXTLINE - downcast.
        auto const& d = static_cast<ReflectionComponentDesc const&> ( desc );

        return std::make_shared<ReflectionComponent> ( renderer, commandBufferConsumed, d, data, commandBuffers );
    }

    commandBufferConsumed = 0U;
    dataRead = sizeof ( ComponentDesc );
    return {};
}

void Component::Register ( lua_State &vm ) noexcept
{
    lua_register ( &vm, "av_ComponentGetName", &Component::OnGetName );
}

Component::Component ( ClassID classID ) noexcept:
    _classID ( classID )
{
    // NOTHING
}

Component::Component ( ClassID classID, std::string &&name ) noexcept:
    _name ( std::move ( name ) ),
    _classID ( classID )
{
    // NOTHING
}

int Component::OnGetName ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::Component::OnGetName - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Component const*> ( lua_touserdata ( state, 1 ) );
    std::string const& n = self._name;
    lua_pushlstring ( state, n.c_str (), n.size () );
    return 1;
}

} // namespace pbr
