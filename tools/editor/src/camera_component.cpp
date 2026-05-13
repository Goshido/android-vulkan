#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <camera_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

CameraComponent::CameraComponent ( MessageQueue &messageQueue ) noexcept:
    Component ( messageQueue, VERSION, "camera" )
{
    // NOTHING
}

CameraComponent::CameraComponent ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept:
    Component ( messageQueue, info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void CameraComponent::Register () noexcept
{
    // FUCK
}

void CameraComponent::Unregister () noexcept
{
    // FUCK
}

void CameraComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
