#include <precompiled_headers.hpp>
#include <sound_emitter_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Factory<SoundEmitterComponent> const SoundEmitterComponent::_factory ( "SoundEmitterComponent" );

SoundEmitterComponent::SoundEmitterComponent () noexcept:
    Component ( "sound emitter" )
{
    // NOTHING
}

SoundEmitterComponent::SoundEmitterComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void SoundEmitterComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
