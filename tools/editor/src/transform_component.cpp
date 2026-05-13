#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <transform_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

TransformComponent::TransformComponent ( MessageQueue &messageQueue ) noexcept:
    Component ( messageQueue, VERSION, "transform" )
{
    // NOTHING
}

TransformComponent::TransformComponent ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept:
    Component ( messageQueue, info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void TransformComponent::Register () noexcept
{
    // FUCK
}

void TransformComponent::Unregister () noexcept
{
    // FUCK
}

void TransformComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
