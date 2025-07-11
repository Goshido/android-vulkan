#include <precompiled_headers.hpp>
#include <point_light_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Factory<PointLightComponent> const PointLightComponent::_factory ( "PointLightComponent" );

PointLightComponent::PointLightComponent () noexcept:
    Component ( "point light" )
{
    // NOTHING
}

PointLightComponent::PointLightComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void PointLightComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
