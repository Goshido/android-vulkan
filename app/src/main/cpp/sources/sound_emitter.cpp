#include <logger.h>
#include <pcm_streamer_ogg.h>
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
    if ( _isPlaying && !Stop () )
        LogWarning ( "SoundEmitter::Destroy - Can't stop." );

    if ( _streamer )
    {
        if ( _streamer->IsDecompressor () )
            _context._soundMixer->UnregisterDecompressor ( *_stream );

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

[[maybe_unused]] std::string const& SoundEmitter::GetFile () const noexcept
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

bool SoundEmitter::IsPlaying () const noexcept
{
    return _isPlaying;
}

bool SoundEmitter::Pause () noexcept
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

bool SoundEmitter::Stop () noexcept
{
    assert ( ( _stream != nullptr ) & static_cast<bool> ( _streamer ) );

    bool const result = SoundMixer::CheckAAudioResult ( AAudioStream_requestStop ( _stream ),
        "SoundEmitter::Stop",
        "Can't stop"
    );

    if ( result )
    {
        if ( !_streamer->Reset () )
            return false;

        _isPlaying = false;
    }

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
    if ( _streamer && _streamer->IsDecompressor () )
        _context._soundMixer->UnregisterDecompressor ( *_stream );

    switch ( GetStreamerType ( file ) )
    {
        case eStreamerType::OGG:
            _streamer = std::make_unique<PCMStreamerOGG> ( *this, &SoundEmitter::OnStopRequested );
        break;

        case eStreamerType::WAV:
            _streamer = std::make_unique<PCMStreamerWAV> ( *this, &SoundEmitter::OnStopRequested );
        break;

        case eStreamerType::UNKNOWN:
            [[fallthrough]];
        default:
        return false;
    }

    if ( !_streamer->SetSoundAsset ( soundStorage, file, looped, _context._soundMixer->GetBufferSampleCount () ) )
    {
        _streamer = nullptr;
        return false;
    }

    _file = file;

    if ( _streamer->IsDecompressor () )
        _context._soundMixer->RegisterDecompressor ( *_stream, *_streamer );

    return true;
}

SoundEmitter::eStreamerType SoundEmitter::GetStreamerType ( std::string_view const asset ) noexcept
{
    auto const findResult = asset.find_last_of ( '.' );

    if ( findResult == std::string_view::npos )
    {
        LogError ( "SoundEmitter::GetStreamerType - Unknown format (branch 1). Asset: %s", asset.data () );
        assert ( false );
        return eStreamerType::UNKNOWN;
    }

    std::string_view const ext = asset.substr ( findResult + 1U );

    if ( ext == "ogg" )
        return eStreamerType::OGG;

    if ( ext == "wav" )
        return eStreamerType::WAV;

    LogError ( "SoundEmitter::GetStreamerType - Unknown format (branch 2). Asset: %s", asset.data () );
    assert ( false );
    return eStreamerType::UNKNOWN;
}

void SoundEmitter::OnStopRequested ( SoundEmitter &soundEmitter ) noexcept
{
    std::thread thread (
        [ &soundEmitter ] () noexcept {
            if ( !soundEmitter.Stop () )
            {
                LogWarning ( "SoundEmitter::OnStopRequested - Can't stop." );
            }
        }
    );

    thread.detach ();
}

} // namespace android_vulkan
