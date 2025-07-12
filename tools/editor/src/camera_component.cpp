#include <precompiled_headers.hpp>
#include <camera_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

CameraComponent::CameraComponent () noexcept:
    Component ( "camera" )
{
    // NOTHING
}

CameraComponent::CameraComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void CameraComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
