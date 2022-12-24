#include <logger.h>
#include <pcm_streamer.h>


namespace android_vulkan {

bool PCMStreamer::SetSoundAsset ( SoundStorage &soundStorage, std::string_view const file, bool looped ) noexcept
{
    auto asset = soundStorage.GetFile ( std::string ( file ) );

    if ( asset == std::nullopt )
        return false;

    _soundFile = *asset;
    auto const result = ResolveInfo ( looped );

    if ( result == std::nullopt )
        return false;

    return IsFormatSupported ( file, *result );
}

PCMStreamer::PCMStreamer ( SoundEmitter &soundEmitter, OnStopRequest callback ) noexcept:
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
