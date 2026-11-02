#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <property_tool.hpp>
#include <sound_emitter_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SoundEmitterComponent::SoundEmitterComponent () noexcept:
    Component ( VERSION, "sound emitter" )
{
    // NOTHING
}

SoundEmitterComponent::SoundEmitterComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void SoundEmitterComponent::Register ( Actor &actor ) noexcept
{
    Component::Register ( actor );
    // FUCK
}

void SoundEmitterComponent::Unregister () noexcept
{
    // FUCK
    Component::Unregister ();
}

void SoundEmitterComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

void SoundEmitterComponent::ShowProperties ( PropertyTool &/*propertyTool*/ ) noexcept
{
    // FUCK
}

} // namespace editor
