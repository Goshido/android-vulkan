#include <precompiled_headers.hpp>
#include <reflection_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ReflectionComponent::ReflectionComponent () noexcept:
    Component ( "reflection" )
{
    // NOTHING
}

ReflectionComponent::ReflectionComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void ReflectionComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
