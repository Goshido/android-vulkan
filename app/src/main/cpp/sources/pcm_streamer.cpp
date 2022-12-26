#include <logger.h>
#include <pcm_streamer.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

void PCMStreamer::OnDecompress () noexcept
{
    // Stub implementation. This code should not be called from decompressor thread. It's an implementation error.
    assert ( false );
}

bool PCMStreamer::Reset () noexcept
{
    _offset = 0U;
    return true;
}

bool PCMStreamer::IsDecompressor () const noexcept
{
    return _decompressor;
}

bool PCMStreamer::SetSoundAsset ( SoundStorage &soundStorage,
    std::string_view const file,
    bool looped,
    size_t samplesPerBurst
) noexcept
{
    auto asset = soundStorage.GetFile ( std::string ( file ) );

    if ( asset == std::nullopt )
        return false;

    _soundFile = *asset;
    auto const result = ResolveInfo ( looped, samplesPerBurst );

    if ( result == std::nullopt || !IsFormatSupported ( file, *result ) )
        return false;

    _offset = 0U;

    bool const isStereo = result->_channelCount > 1U;
    constexpr ReadHandler const readHandlers[] = { &PCMStreamer::HandleMono, &PCMStreamer::HandleStereo };
    _readHandler = readHandlers[ static_cast<size_t> ( isStereo ) ];

    constexpr LoopHandler const loopHandlers[] =
    {
        &PCMStreamer::HandleNonLooped,
        &PCMStreamer::HandleNonLooped,
        &PCMStreamer::HandleLoopedMono,
        &PCMStreamer::HandleLoopedStereo
    };

    _loopHandler = loopHandlers[ ( static_cast<size_t> ( looped ) << 1U ) | static_cast<size_t> ( isStereo ) ];
    return true;
}

void PCMStreamer::HandleNonLooped ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* /*pcm*/,
    Consume consume,
    int32_t /*leftGain*/,
    int32_t /*rightGain*/
) noexcept
{
    size_t const rest = bufferSamples - consume._bufferSampleCount;
    _offset += consume._pcmSampleCount;

    if ( rest > 0U )
    {
        std::memset ( buffer + ( bufferSamples - rest ), 0, rest * sizeof ( PCMType ) );
        return;
    }

    if ( _offset < _sampleCount )
        return;

    _offset = 0U;

    if ( consume._lastPCMBuffer )
    {
        _onStopRequest ( _soundEmitter );
    }
}

PCMStreamer::PCMStreamer ( SoundEmitter &soundEmitter, OnStopRequest callback, bool decompressor ) noexcept:
    _decompressor ( decompressor ),
    _onStopRequest ( callback ),
    _soundEmitter ( soundEmitter )
{
    // NOTHING
}

bool PCMStreamer::IsFormatSupported ( std::string_view const file, Info const &info ) noexcept
{
    if ( info._channelCount == 0U | info._channelCount > 2U )
    {
        LogError ( "PCMStreamer::IsFormatSupported - Unsupported %hhu channel count. Please re-encode sound as "
            "16 bit mono|stereo signed. File: %s",
            info._channelCount,
            file.data ()
        );

        return false;
    }

    if ( info._channelCount == 1U & info._bytesPerChannelSample != 2U )
    {
        LogError ( "PCMStreamer::IsFormatSupported - %u bits mono sound is not supported. Please re-encode sound as "
            "signed 16 bits mono. File: %s",
            8U * static_cast<uint32_t> ( info._bytesPerChannelSample ),
            file.data ()
        );

        return false;
    }

    if ( info._channelCount == 2U & info._bytesPerChannelSample != 2U )
    {
        LogError ( "PCMStreamer::IsFormatSupported - %u bits stereo sound is not supported. Please re-encode sound as "
            "signed 16 bits stereo. File: %s",
            8U * static_cast<uint32_t> ( info._bytesPerChannelSample ),
            file.data ()
        );

        return false;
    }

    if ( ( info._channelCount == 1U | info._channelCount == 2U ) & ( info._bytesPerChannelSample == 2U ) )
        return true;

    constexpr char const format[] =
R"__(PCMStreamer::IsFormatSupported - Unsupported sound format.
    Channels: %hhu,
    Bytes per channel sample: %hhu,
    Sample rate: %u,
    File: %s)__";

    LogError ( format, info._channelCount, info._bytesPerChannelSample, info._sampleRate,file.data () );
    return true;
}

} // namespace android_vulkan
