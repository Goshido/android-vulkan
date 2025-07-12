#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <camera_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

CameraComponent::CameraComponent () noexcept:
    Component ( VERSION, "camera" )
{
    // NOTHING
}

CameraComponent::CameraComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void CameraComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
