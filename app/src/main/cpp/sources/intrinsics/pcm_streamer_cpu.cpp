#include <logger.h>
#include <pcm_streamer_wav.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

constexpr auto DENOMINATOR = static_cast<int32_t> ( PCMStreamer::INTEGER_DIVISION_SCALE );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void PCMStreamer::HandleLoopedMono ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    Consume consume,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const restInBufferSamples = bufferSamples - consume._bufferSampleCount;
    _offset = ( _offset + consume._pcmSampleCount ) % _sampleCount;

    if ( restInBufferSamples == 0U )
        return;

    buffer += bufferSamples - restInBufferSamples;
    size_t const restInPCMSamples = restInBufferSamples >> 1U;

    for ( size_t i = 0U; i < restInPCMSamples; ++i )
    {
        size_t const leftIdx = i << 1U;
        size_t const rightIdx = leftIdx + 1U;

        auto const sample = static_cast<int32_t> ( pcm[ i ] );

        buffer[ leftIdx ] = static_cast<PCMType> ( sample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( sample * rightGain / DENOMINATOR );
    }

    _offset = restInPCMSamples;
}

void PCMStreamer::HandleLoopedStereo ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    Consume consume,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const rest = bufferSamples - consume._bufferSampleCount;
    _offset = ( _offset + consume._pcmSampleCount ) % _sampleCount;

    if ( rest == 0U )
        return;

    buffer += bufferSamples - rest;

    for ( size_t i = 0U; i < rest; i += 2U )
    {
        size_t const rightIdx = i + 1U;

        auto const leftSample = static_cast<int32_t> ( pcm[ i ] );
        auto const rightSample = static_cast<int32_t> ( pcm[ rightIdx ] );

        buffer[ i ] = static_cast<PCMType> ( leftSample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( rightSample * rightGain / DENOMINATOR );
    }

    _offset = rest;
}

PCMStreamer::Consume PCMStreamer::HandleMono ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    bool lastPCMBuffer,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const bufferFrames = bufferSamples >> 1U;
    size_t const cases[] = { bufferFrames, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferFrames > _sampleCount ) ];

    for ( size_t i = 0U; i < canRead; ++i )
    {
        size_t const leftIdx = i << 1U;
        size_t const rightIdx = leftIdx + 1U;

        auto const sample = static_cast<int32_t> ( pcm[ i ] );

        buffer[ leftIdx ] = static_cast<PCMType> ( sample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( sample * rightGain / DENOMINATOR );
    }

    return Consume
    {
        ._bufferSampleCount = canRead << 1U,
        ._lastPCMBuffer = lastPCMBuffer,
        ._pcmSampleCount = canRead
    };
}

PCMStreamer::Consume PCMStreamer::HandleStereo ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    bool lastPCMBuffer,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const cases[] = { bufferSamples, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferSamples > _sampleCount ) ];

    for ( size_t i = 0U; i < canRead; i += 2U )
    {
        size_t const rightIdx = i + 1U;

        auto const leftSample = static_cast<int32_t> ( pcm[ i ] );
        auto const rightSample = static_cast<int32_t> ( pcm[ rightIdx ] );

        buffer[ i ] = static_cast<PCMType> ( leftSample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( rightSample * rightGain / DENOMINATOR );
    }

    return Consume
    {
        ._bufferSampleCount = canRead,
        ._lastPCMBuffer = lastPCMBuffer,
        ._pcmSampleCount = canRead
    };
}

} // namespace android_vulkan
