#include <precompiled_headers.hpp>
#include <script_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ScriptComponent::ScriptComponent () noexcept:
    Component ( "script" )
{
    // NOTHING
}

ScriptComponent::ScriptComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void ScriptComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
