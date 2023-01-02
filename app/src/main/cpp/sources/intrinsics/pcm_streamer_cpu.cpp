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

void PCMStreamer::HandleLoopedMono ( std::span<PCMType> buffer,
    PCMType const* pcm,
    Consume consume,
    Gain leftGain,
    Gain rightGain
) noexcept
{
    size_t const bufferSamples = buffer.size ();
    size_t const restInBufferSamples = bufferSamples - consume._bufferSampleCount;
    _offset = ( _offset + consume._pcmSampleCount ) % _sampleCount;

    if ( restInBufferSamples == 0U )
        return;

    PCMType* target = buffer.data () + ( bufferSamples - restInBufferSamples );
    size_t const restInPCMSamples = restInBufferSamples >> 1U;

    int32_t const leftDiff = leftGain._current - leftGain._before;
    int32_t const rightDiff = rightGain._current - rightGain._before;

    for ( size_t i = 0U; i < restInPCMSamples; ++i )
    {
        int32_t const alpha = static_cast<int32_t> ( i ) * DENOMINATOR / static_cast<int32_t> ( restInPCMSamples );
        int32_t const l = leftGain._before + leftDiff * alpha / DENOMINATOR;
        int32_t const r = rightGain._before + rightDiff * alpha / DENOMINATOR;

        size_t const leftIdx = i << 1U;
        size_t const rightIdx = leftIdx + 1U;

        auto const sample = static_cast<int32_t> ( pcm[ i ] );
        target[ leftIdx ] = static_cast<PCMType> ( sample * l / DENOMINATOR );
        target[ rightIdx ] = static_cast<PCMType> ( sample * r / DENOMINATOR );
    }

    _offset = restInPCMSamples;
}

void PCMStreamer::HandleLoopedStereo ( std::span<PCMType> buffer,
    PCMType const* pcm,
    Consume consume,
    Gain leftGain,
    Gain rightGain
) noexcept
{
    size_t const bufferSamples = buffer.size ();
    size_t const rest = bufferSamples - consume._bufferSampleCount;
    _offset = ( _offset + consume._pcmSampleCount ) % _sampleCount;

    if ( rest == 0U )
        return;

    PCMType* target = buffer.data () + ( bufferSamples - rest );

    int32_t const leftDiff = leftGain._current - leftGain._before;
    int32_t const rightDiff = rightGain._current - rightGain._before;

    for ( size_t i = 0U; i < rest; i += 2U )
    {
        int32_t const alpha = static_cast<int32_t> ( i ) * DENOMINATOR / static_cast<int32_t> ( rest );
        int32_t const l = leftGain._before + leftDiff * alpha / DENOMINATOR;
        int32_t const r = rightGain._before + rightDiff * alpha / DENOMINATOR;

        size_t const rightIdx = i + 1U;

        auto const leftSample = static_cast<int32_t> ( pcm[ i ] );
        auto const rightSample = static_cast<int32_t> ( pcm[ rightIdx ] );

        target[ i ] = static_cast<PCMType> ( leftSample * l / DENOMINATOR );
        target[ rightIdx ] = static_cast<PCMType> ( rightSample * r / DENOMINATOR );
    }

    _offset = rest;
}

PCMStreamer::Consume PCMStreamer::HandleMono ( std::span<PCMType> buffer,
    Gain &leftGain,
    Gain &rightGain,
    PCMType const* pcm,
    bool lastPCMBuffer
) const noexcept
{
    size_t const bufferFrames = buffer.size () >> 1U;
    size_t const cases[] = { bufferFrames, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferFrames > _sampleCount ) ];

    int32_t const leftDiff = leftGain._current - leftGain._before;
    int32_t const rightDiff = rightGain._current - rightGain._before;

    for ( size_t i = 0U; i < canRead; ++i )
    {
        int32_t const alpha = static_cast<int32_t> ( i ) * DENOMINATOR / static_cast<int32_t> ( bufferFrames );
        int32_t const l = leftGain._before + leftDiff * alpha / DENOMINATOR;
        int32_t const r = rightGain._before + rightDiff * alpha / DENOMINATOR;

        size_t const leftIdx = i << 1U;
        size_t const rightIdx = leftIdx + 1U;

        auto const sample = static_cast<int32_t> ( pcm[ i ] );

        buffer[ leftIdx ] = static_cast<PCMType> ( sample * l / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( sample * r / DENOMINATOR );
    }

    int32_t const alpha = static_cast<int32_t> ( canRead ) * DENOMINATOR / static_cast<int32_t> ( bufferFrames );
    leftGain._current = leftGain._before + leftDiff * alpha / DENOMINATOR;
    rightGain._current = rightGain._before + rightDiff * alpha / DENOMINATOR;

    return Consume
    {
        ._bufferSampleCount = canRead << 1U,
        ._lastPCMBuffer = lastPCMBuffer,
        ._pcmSampleCount = canRead
    };
}

PCMStreamer::Consume PCMStreamer::HandleStereo ( std::span<PCMType> buffer,
    Gain &leftGain,
    Gain &rightGain,
    PCMType const* pcm,
    bool lastPCMBuffer
) const noexcept
{
    size_t const bufferSamples = buffer.size ();
    size_t const cases[] = { bufferSamples, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferSamples > _sampleCount ) ];

    int32_t const leftDiff = leftGain._current - leftGain._before;
    int32_t const rightDiff = rightGain._current - rightGain._before;

    for ( size_t i = 0U; i < canRead; i += 2U )
    {
        int32_t const alpha = static_cast<int32_t> ( i ) * DENOMINATOR / static_cast<int32_t> ( bufferSamples );
        int32_t const l = leftGain._before + leftDiff * alpha / DENOMINATOR;
        int32_t const r = rightGain._before + rightDiff * alpha / DENOMINATOR;

        size_t const rightIdx = i + 1U;

        auto const leftSample = static_cast<int32_t> ( pcm[ i ] );
        auto const rightSample = static_cast<int32_t> ( pcm[ rightIdx ] );

        buffer[ i ] = static_cast<PCMType> ( leftSample * l / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( rightSample * r / DENOMINATOR );
    }

    int32_t const alpha = static_cast<int32_t> ( canRead ) * DENOMINATOR / static_cast<int32_t> ( bufferSamples );
    leftGain._current = leftGain._before + leftDiff * alpha / DENOMINATOR;
    rightGain._current = rightGain._before + rightDiff * alpha / DENOMINATOR;

    return Consume
    {
        ._bufferSampleCount = canRead,
        ._lastPCMBuffer = lastPCMBuffer,
        ._pcmSampleCount = canRead
    };
}

} // namespace android_vulkan
