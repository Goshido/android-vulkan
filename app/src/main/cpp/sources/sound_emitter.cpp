#include <logger.h>
#include <pcm_streamer_wav.h>
#include <sound_emitter.h>
#include <sound_mixer.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

[[maybe_unused]] bool SoundEmitter::Init ( SoundMixer &soundMixer, eSoundChannel channel ) noexcept
{
    _context =
    {
        ._soundMixer = &soundMixer,
        ._soundEmitter = this
    };

    _channel = channel;

    if ( auto const stream = soundMixer.CreateStream ( _context ); stream != std::nullopt )
    {
        _stream = *stream;
        return true;
    }

    return false;
}

[[maybe_unused]] bool SoundEmitter::Destroy () noexcept
{
    if ( _streamer )
        _streamer = nullptr;

    if ( !_stream )
        return true;

    bool const result = SoundMixer::CheckAAudioResult ( AAudioStream_close ( _stream ),
        "SoundEmitter::Destroy",
        "Can't close stream"
    );

    if ( !result )
        return false;

    _stream = nullptr;
    return true;
}

[[maybe_unused]] bool SoundEmitter::SetSoundAsset ( SoundStorage &soundStorage, std::string_view const file ) noexcept
{
    switch ( GetStreamerType ( file ) )
    {
        case eStreamerType::OGG:
            // TODO
            assert ( false );
        break;

        case eStreamerType::WAV:
            _streamer = std::make_unique<PCMStreamerWAV> ();
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    if ( _streamer->SetSoundAsset ( soundStorage, file, _context._soundMixer->GetBufferFrameCount () ) )
        return true;

    _streamer = nullptr;
    return false;
}

SoundEmitter::eStreamerType SoundEmitter::GetStreamerType ( std::string_view const /*asset*/ ) noexcept
{
    // TODO
    return eStreamerType::WAV;
}

} // namespace android_vulkan
