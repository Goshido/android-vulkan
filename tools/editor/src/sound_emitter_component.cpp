#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <sound_emitter_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SoundEmitterComponent::SoundEmitterComponent ( MessageQueue &messageQueue ) noexcept:
    Component ( messageQueue, VERSION, "sound emitter" )
{
    // NOTHING
}

SoundEmitterComponent::SoundEmitterComponent ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept:
    Component ( messageQueue, info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void SoundEmitterComponent::Register () noexcept
{
    // FUCK
}

void SoundEmitterComponent::Unregister () noexcept
{
    // FUCK
}

void SoundEmitterComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
