#include <logger.h>
#include <pcm_streamer_wav.h>


namespace android_vulkan {

namespace {

#pragma pack ( push, 1 )

struct WAVEHeader final
{
    [[maybe_unused]] char           _chunkId[ 4U ];
    [[maybe_unused]] uint32_t       _chunkSize;

    [[maybe_unused]] char           _format[ 4U ];
    [[maybe_unused]] char           _fmtChunkId[ 4U ];
    [[maybe_unused]] uint32_t       _fmtChunkSize;

    [[maybe_unused]] uint16_t       _audioFormat;
    uint16_t                        _numChannels;
    uint32_t                        _sampleRate;
    [[maybe_unused]] uint32_t       _byteRate;
    [[maybe_unused]] uint16_t       _blockAlign;
    uint16_t                        _bitsPerSample;

    [[maybe_unused]] char           _dataChunkId[ 4U ];
    uint32_t                        _dataChunkSize;
};

#pragma pack ( pop )

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

PCMStreamerWAV::PCMStreamerWAV ( SoundEmitter &soundEmitter, OnStopRequest callback ) noexcept:
    PCMStreamer ( soundEmitter, callback, false )
{
    // NOTHING
}

void PCMStreamerWAV::GetNextBuffer ( std::span<PCMType> buffer,
    float leftChannelVolume,
    float rightChannelVolume
) noexcept
{
    auto const leftGain = static_cast<int32_t> ( leftChannelVolume * static_cast<float> ( INTEGER_DIVISION_SCALE ) );
    auto const rightGain = static_cast<int32_t> ( rightChannelVolume * static_cast<float> ( INTEGER_DIVISION_SCALE ) );

    PCMType* target = buffer.data ();
    auto const* pcm = reinterpret_cast<PCMType const*> ( _soundFile->data () + sizeof ( WAVEHeader ) );
    size_t const bufferSamples = buffer.size ();

    if ( bufferSamples > _sampleCount )
    {
        // C++ calling method by pointer syntax.
        ( this->*_loopHandler ) ( target,
            bufferSamples,
            pcm,

            Consume
            {
                ._bufferSampleCount = 0U,
                ._lastPCMBuffer = true,
                ._pcmSampleCount = 0U
            },

            leftGain,
            rightGain
        );

        return;
    }

    // C++ calling method by pointer syntax.
    Consume const consume = ( this->*_readHandler ) ( target,
        bufferSamples,
        pcm + _offset,
        true,
        leftGain,
        rightGain
    );

    // C++ calling method by pointer syntax.
    ( this->*_loopHandler ) ( target, bufferSamples, pcm, consume, leftGain, rightGain );
}

std::optional<PCMStreamer::Info> PCMStreamerWAV::ResolveInfo ( bool /*looped*/, size_t /*samplesPerBurst*/ ) noexcept
{
    auto const& header = *reinterpret_cast<WAVEHeader const*> ( _soundFile->data () );
    _sampleCount = static_cast<size_t> ( header._dataChunkSize ) / sizeof ( PCMType );

    return Info
    {
        ._bytesPerChannelSample =  static_cast<uint8_t> ( header._bitsPerSample / 8U ),
        ._channelCount = static_cast<uint8_t> ( header._numChannels ),
        ._sampleRate = header._sampleRate
    };
}

} // namespace android_vulkan
