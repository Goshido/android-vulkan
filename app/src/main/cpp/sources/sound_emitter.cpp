#include <av_assert.hpp>
#include <logger.hpp>
#include <pcm_streamer_ogg.hpp>
#include <pcm_streamer_wav.hpp>
#include <sound_emitter.hpp>
#include <sound_mixer.hpp>


namespace android_vulkan {

void SoundEmitter::Init ( SoundMixer &soundMixer, eSoundChannel channel ) noexcept
{
    _channel = channel;
    _mixer = &soundMixer;
}

bool SoundEmitter::Destroy () noexcept
{
    if ( _isPlaying && !Stop () )
        LogWarning ( "SoundEmitter::Destroy - Can't stop." );

    if ( _streamer )
    {
        if ( _streamer->IsDecompressor () )
            _mixer->UnregisterDecompressor ( *_streamer );

        _file.clear ();
        _streamer = nullptr;
    }

    _mixer = nullptr;
    return true;
}

[[maybe_unused]] std::string const &SoundEmitter::GetFile () const noexcept
{
    return _file;
}

eSoundChannel SoundEmitter::GetSoundChannel () const noexcept
{
    return _channel;
}

float SoundEmitter::GetVolume () const noexcept
{
    return _volume;
}

void SoundEmitter::SetVolume ( float volume ) noexcept
{
    _volume = std::clamp ( volume, 0.0F, 1.0F );
}

bool SoundEmitter::IsPlaying () const noexcept
{
    return _isPlaying;
}

bool SoundEmitter::Pause () noexcept
{
    AV_ASSERT ( ( _mixer != nullptr ) & static_cast<bool> ( _streamer ) )

    if ( _mixer->RequestPause ( *this ) )
    {
        _isPlaying = false;
        return true;
    }

    return false;
}

bool SoundEmitter::Play () noexcept
{
    AV_ASSERT ( ( _mixer != nullptr ) & static_cast<bool> ( _streamer ) )

    if ( _mixer->RequestPlay ( *this ) )
    {
        _isPlaying = true;
        return false;
    }

    return false;
}

bool SoundEmitter::Stop () noexcept
{
    AV_ASSERT ( ( _mixer != nullptr ) & static_cast<bool> ( _streamer ) )

    if ( !_isPlaying )
        return true;

    if ( _mixer->RequestStop ( *this ) )
    {
        if ( !_streamer->Reset () )
            return false;

        _isPlaying = false;
        return true;
    }

    return false;
}

bool SoundEmitter::SetSoundAsset ( std::string_view const file, bool looped ) noexcept
{
    if ( _streamer && _streamer->IsDecompressor () )
        _mixer->UnregisterDecompressor ( *_streamer );

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

    bool const result = _streamer->SetSoundAsset ( _mixer->GetSoundStorage (),
        file,
        looped,
        _mixer->GetBufferSampleCount ()
    );

    if ( !result )
    {
        _streamer = nullptr;
        return false;
    }

    _file = file;

    if ( _streamer->IsDecompressor () )
        _mixer->RegisterDecompressor ( *_streamer );

    return true;
}

SoundEmitter::eStreamerType SoundEmitter::GetStreamerType ( std::string_view const asset ) noexcept
{
    auto const findResult = asset.find_last_of ( '.' );

    if ( findResult == std::string_view::npos )
    {
        LogError ( "SoundEmitter::GetStreamerType - Unknown format (branch 1). Asset: %s", asset.data () );
        AV_ASSERT ( false )
        return eStreamerType::UNKNOWN;
    }

    std::string_view const ext = asset.substr ( findResult + 1U );

    if ( ext == "ogg" )
        return eStreamerType::OGG;

    if ( ext == "wav" )
        return eStreamerType::WAV;

    LogError ( "SoundEmitter::GetStreamerType - Unknown format (branch 2). Asset: %s", asset.data () );
    AV_ASSERT ( false )
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
