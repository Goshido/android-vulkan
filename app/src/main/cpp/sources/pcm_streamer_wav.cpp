#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pcm_streamer_wav.hpp>


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

void PCMStreamerWAV::GetNextBuffer ( std::span<PCMType> buffer, Gain left, Gain right ) noexcept
{
    auto const* pcm = reinterpret_cast<PCMType const*> ( _soundFile->data () + sizeof ( WAVEHeader ) );

    if ( buffer.size () > _sampleCount )
    {
        // C++ calling method by pointer syntax.
        ( this->*_loopHandler ) ( buffer,
            pcm,

            Consume
            {
                ._bufferSampleCount = 0U,
                ._lastPCMBuffer = true,
                ._pcmSampleCount = 0U
            },

            left,
            right
        );

        return;
    }

    // C++ calling method by pointer syntax.
    Consume const consume = ( this->*_readHandler ) ( buffer,
        left,
        right,
        pcm + _offset,
        true
    );

    // C++ calling method by pointer syntax.
    ( this->*_loopHandler ) ( buffer, pcm, consume, left, right );
}

std::optional<PCMStreamer::Info> PCMStreamerWAV::ResolveInfo ( bool /*looped*/, size_t /*samplesPerBurst*/ ) noexcept
{
    auto const &header = *reinterpret_cast<WAVEHeader const*> ( _soundFile->data () );
    _sampleCount = static_cast<size_t> ( header._dataChunkSize ) / sizeof ( PCMType );

    return Info
    {
        ._bytesPerChannelSample =  static_cast<uint8_t> ( header._bitsPerSample / 8U ),
        ._channelCount = static_cast<uint8_t> ( header._numChannels ),
        ._sampleRate = header._sampleRate
    };
}

} // namespace android_vulkan
