#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <rigid_body_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

RigidBodyComponent::RigidBodyComponent ( MessageQueue &messageQueue ) noexcept:
    Component ( messageQueue, VERSION, "rigid body" )
{
    // NOTHING
}

RigidBodyComponent::RigidBodyComponent ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept:
    Component ( messageQueue, info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void RigidBodyComponent::Register () noexcept
{
    // FUCK
}

void RigidBodyComponent::Unregister () noexcept
{
    // FUCK
}

void RigidBodyComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
