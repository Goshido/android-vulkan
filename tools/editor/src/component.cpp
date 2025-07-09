#include <precompiled_headers.hpp>
#include <component.hpp>


namespace editor {

namespace {

//constexpr std::string_view NAME_KEY = "name";
//constexpr std::string_view DEFAULT_NAME = "component";

//constexpr std::string_view TYPE_KEY = "type";
//constexpr std::string_view PARENT_KEY = "parent";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Component () noexcept
{
    _local.Identity ();
    _parent.Identity ();
}

Component::Component ( SaveState::Container const &/*info*/ ) noexcept
{
    // FUCK
}

void Component::Save ( SaveState::Container &/*root*/ ) const noexcept
{
    // FUCK
}

std::optional<Component::Ref> Component::Spawn ( SaveState::Container const &/*info*/ ) noexcept
{
    // FUCK
    return std::nullopt;
}

} // namespace editor
