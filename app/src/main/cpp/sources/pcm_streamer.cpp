#include <pcm_streamer.h>


namespace android_vulkan {

[[maybe_unused]] uint8_t PCMStreamer::GetChannelCount () const noexcept
{
    return _channelCount;
}

[[maybe_unused]] bool PCMStreamer::SetSoundAsset ( SoundStorage &soundStorage,
    std::string &&file,
    size_t bufferLengthMs
) noexcept
{
    auto asset = soundStorage.GetFile ( std::move ( file ) );

    if ( asset == std::nullopt )
        return false;

    _soundFile = *asset;
    auto const info = ResolveInfo ();

    if ( info == std::nullopt )
        return false;

    _activeBuffer = 0U;

    _channelCount = info->_channelCount;
    size_t const sizePerSecond = info->_channelCount * info->_sampleRate * sizeof ( PCMType );

    constexpr size_t millisecondsPerSecond = 1000U;
    size_t const size = sizePerSecond * bufferLengthMs / millisecondsPerSecond;

    for ( auto& buffer : _buffers )
        buffer.resize ( size );

    return true;
}

} // namespace android_vulkan
