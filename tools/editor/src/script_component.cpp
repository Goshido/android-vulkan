#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <script_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ScriptComponent::ScriptComponent ( MessageQueue &messageQueue ) noexcept:
    Component ( messageQueue, VERSION, "script" )
{
    // NOTHING
}

ScriptComponent::ScriptComponent ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept:
    Component ( messageQueue, info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void ScriptComponent::Register () noexcept
{
    // FUCK
}

void ScriptComponent::Unregister () noexcept
{
    // FUCK
}

void ScriptComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
