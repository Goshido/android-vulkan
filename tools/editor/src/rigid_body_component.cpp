#include <precompiled_headers.hpp>
#include <rigid_body_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Factory<RigidBodyComponent> const RigidBodyComponent::_factory ( "RigidBodyComponent" );

RigidBodyComponent::RigidBodyComponent () noexcept:
    Component ( "rigid body" )
{
    // NOTHING
}

RigidBodyComponent::RigidBodyComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void RigidBodyComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
