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
        ._soundChannel = channel,
        ._soundMixer = &soundMixer,
        ._soundEmitter = this
    };

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
    {
        _file.clear ();
        _streamer = nullptr;
    }

    if ( !_stream )
        return true;

    if ( bool const result = _context._soundMixer->DestroyStream ( *_stream ); !result )
        return false;

    _stream = nullptr;
    return true;
}

SoundEmitter::Context& SoundEmitter::GetContext () noexcept
{
    return _context;
}

std::string const& SoundEmitter::GetFile () const noexcept
{
    return _file;
}

[[maybe_unused]] float SoundEmitter::GetVolume () const noexcept
{
    return _volume;
}

[[maybe_unused]] void SoundEmitter::SetVolume ( float volume ) noexcept
{
    _volume = std::clamp ( volume, 0.0F, 1.0F );
}

[[maybe_unused]] bool SoundEmitter::Pause () noexcept
{
    assert ( ( _stream != nullptr ) & static_cast<bool> ( _streamer ) );

    bool const result = SoundMixer::CheckAAudioResult ( AAudioStream_requestPause ( _stream ),
        "SoundEmitter::Pause",
        "Can't pause"
    );

    if ( result )
        _isPlaying = false;

    return result;
}

bool SoundEmitter::Play () noexcept
{
    assert ( ( _stream != nullptr ) & static_cast<bool> ( _streamer ) );

    bool const result =  SoundMixer::CheckAAudioResult ( AAudioStream_requestStart ( _stream ),
        "SoundEmitter::Play",
        "Can't play"
    );

    if ( result )
        _isPlaying = true;

    return result;
}

[[maybe_unused]] bool SoundEmitter::Stop () noexcept
{
    assert ( ( _stream != nullptr ) & static_cast<bool> ( _streamer ) );

    bool const result = SoundMixer::CheckAAudioResult ( AAudioStream_requestStop ( _stream ),
        "SoundEmitter::Stop",
        "Can't stop"
    );

    if ( result )
        _isPlaying = false;

    return result;
}

void SoundEmitter::OnStreamRecreated ( AAudioStream &stream ) noexcept
{
    _stream = &stream;

    if ( !_isPlaying )
        return;

    if ( !Play () )
    {
        LogWarning ( "SoundEmitter::OnStreamRecreated - Can't start playing." );
    }
}

[[maybe_unused]] bool SoundEmitter::SetSoundAsset ( SoundStorage &soundStorage,
    std::string_view const file,
    bool looped
) noexcept
{
    switch ( GetStreamerType ( file ) )
    {
        case eStreamerType::OGG:
            // TODO
            assert ( false );
        break;

        case eStreamerType::WAV:
            _streamer = std::make_unique<PCMStreamerWAV> ( *this, &SoundEmitter::OnStopRequested );
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    if ( _streamer->SetSoundAsset ( soundStorage, file, looped ) )
    {
        _file = file;
        return true;
    }

    _streamer = nullptr;
    return false;
}

SoundEmitter::eStreamerType SoundEmitter::GetStreamerType ( std::string_view const /*asset*/ ) noexcept
{
    // TODO
    return eStreamerType::WAV;
}

void SoundEmitter::OnStopRequested ( SoundEmitter &soundEmitter ) noexcept
{
    if ( !soundEmitter.Stop () )
    {
        LogWarning ( "SoundEmitter::OnStopRequested - Can't stop." );
    }
}

} // namespace android_vulkan
