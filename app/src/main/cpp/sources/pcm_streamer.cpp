#include <logger.h>
#include <pcm_streamer.h>


namespace android_vulkan {

[[maybe_unused]] uint8_t PCMStreamer::GetChannelCount () const noexcept
{
    return _channelCount;
}

bool PCMStreamer::SetSoundAsset ( SoundStorage &soundStorage,
    std::string_view const file,
    size_t bufferFrames
) noexcept
{
    auto asset = soundStorage.GetFile ( std::string ( file ) );

    if ( asset == std::nullopt )
        return false;

    _soundFile = *asset;
    auto const result = ResolveInfo ();

    if ( result == std::nullopt )
        return false;

    Info const &info = *result;

    if ( !IsFormatSupported ( file, info ) )
        return false;

    _activeBuffer = 0U;
    _channelCount = info._channelCount;
    size_t const size = info._channelCount * bufferFrames;

    for ( auto& buffer : _buffers )
        buffer.resize ( size );

    return true;
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
