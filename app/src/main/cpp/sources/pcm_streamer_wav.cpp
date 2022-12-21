#include <pcm_streamer_wav.h>


namespace android_vulkan {

[[maybe_unused]] std::span<PCMStreamer::PCMType const> PCMStreamerWAV::GetNextBuffer () noexcept
{
    // TODO
    return {};
}

std::optional<PCMStreamer::Info> PCMStreamerWAV::ResolveInfo () noexcept
{

#pragma pack ( push, 1 )

    struct WAVEHeader final
    {
        char        _chunkId[ 4U ];
        uint32_t    _chunkSize;

        char        _format[ 4U ];
        char        _fmtChunkId[ 4U ];
        uint32_t    _fmtChunkSize;

        uint16_t    _audioFormat;
        uint16_t    _numChannels;
        uint32_t    _sampleRate;
        uint32_t    _byteRate;
        uint16_t    _blockAlign;
        uint16_t    _bitsPerSample;

        char        _dataChunkId[ 4U ];
        uint32_t    _dataChunkSize;
    };

#pragma pack ( pop )

    auto const& header = *reinterpret_cast<WAVEHeader const*> ( _soundFile->data () );

    return Info
    {
        ._bytesPerChannelSample =  static_cast<uint8_t> ( header._bitsPerSample / 8U ),
        ._channelCount = static_cast<uint8_t> ( header._numChannels ),
        ._sampleRate = header._sampleRate
    };
}

} // namespace android_vulkan
