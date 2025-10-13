#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <reflection_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ReflectionComponent::ReflectionComponent () noexcept:
    Component ( VERSION, "reflection" )
{
    // NOTHING
}

ReflectionComponent::ReflectionComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void ReflectionComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
