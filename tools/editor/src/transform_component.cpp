#include <precompiled_headers.hpp>
#include <transform_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Factory<TransformComponent> const TransformComponent::_factory ( "TransformComponent" );

TransformComponent::TransformComponent () noexcept:
    Component ( "transform" )
{
    // NOTHING
}

TransformComponent::TransformComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void TransformComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
