#include <logger.h>
#include <pcm_streamer_wav.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <arm_neon.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

// Must be power of two because Right bit shift is used under the hood.
constexpr auto DENOMINATOR = static_cast<int32_t> ( PCMStreamer::INTEGER_DIVISION_SCALE );

constexpr size_t IN_NEON_REGISTERS = 4U;
constexpr size_t SAMPLES_PER_REGISTER = 4U;
constexpr size_t STEREO_SAMPLES_PER_ITERATION = IN_NEON_REGISTERS * SAMPLES_PER_REGISTER;
constexpr size_t MONO_SAMPLES_PER_ITERATION = STEREO_SAMPLES_PER_ITERATION / 2U;

//----------------------------------------------------------------------------------------------------------------------

// Neon does not support integer division. But we could achieve similar result by bit shifting and
// taking into consideration sign bit.
// https://tttapa.github.io/Pages/Raspberry-Pi/NEON/Division.html
class NeonConverter final
{
    public:
        NeonConverter () = delete;

        NeonConverter ( NeonConverter const & ) = delete;
        NeonConverter& operator = ( NeonConverter const & ) = delete;

        NeonConverter ( NeonConverter && ) = delete;
        NeonConverter& operator = ( NeonConverter && ) = delete;

        ~NeonConverter () = delete;

        static void Convert ( PCMStreamer::PCMType* target,
            int32_t* scratchPad,
            int32x4_t const &nominator,
            int32x4_t const &correction
        ) noexcept;

        [[nodiscard]] static int32x4_t MakeCorrectionVector () noexcept;
        [[nodiscard]] static int32x4_t MakeNominator ( int32_t leftGain, int32_t rightGain ) noexcept;

    private:
        [[nodiscard]] static int32x4_t Divide ( int32x4_t v, int32x4_t const &correction ) noexcept;
};

void NeonConverter::Convert ( PCMStreamer::PCMType* target,
    int32_t* scratchPad,
    int32x4_t const &nominator,
    int32x4_t const &correction
) noexcept
{
    int32x4_t const s0 = vld1q_s32 ( scratchPad );
    int32x4_t const a0 = vmulq_s32 ( s0, nominator );

    int32x4_t const s1 = vld1q_s32 ( scratchPad + 4U );
    int32x4_t const t0 = NeonConverter::Divide ( a0, correction );

    int32x4_t const s2 = vld1q_s32 ( scratchPad + 8U );
    int32x4_t const a1 = vmulq_s32 ( s1, nominator );

    int32x4_t const s3 = vld1q_s32 ( scratchPad + 12U );
    int32x4_t const t1 = NeonConverter::Divide ( a1, correction );
    int32x4_t const a2 = vmulq_s32 ( s2, nominator );

    vst1q_s32 ( scratchPad, t0 );
    int32x4_t const a3 = vmulq_s32 ( s3, nominator );

    vst1q_s32 ( scratchPad + 4U, t1 );
    int32x4_t const t2 = NeonConverter::Divide ( a2, correction );

    int32x4_t const t3 = NeonConverter::Divide ( a3, correction );
    vst1q_s32 ( scratchPad + 8U, t2 );
    vst1q_s32 ( scratchPad + 12U, t3 );

    for ( size_t idx = 0U; idx < STEREO_SAMPLES_PER_ITERATION; ++idx )
    {
        target[ idx ] = static_cast<PCMStreamer::PCMType>( scratchPad[ idx ] );
    }
}

int32x4_t NeonConverter::Divide ( int32x4_t v, int32x4_t const &correction ) noexcept
{
    return vshrq_n_s32 ( vaddq_s32 ( v, vandq_s32 ( vcltzq_s32 ( v ), correction ) ), 15 );
}

int32x4_t NeonConverter::MakeCorrectionVector () noexcept
{
    return vdupq_n_s32 ( DENOMINATOR - 1 );
}

int32x4_t NeonConverter::MakeNominator ( int32_t leftGain, int32_t rightGain ) noexcept
{
    int32_t const nominatorData[] = { leftGain, rightGain, leftGain, rightGain };
    return vld1q_s32 ( nominatorData );
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void PCMStreamerWAV::HandleLoopedStereo ( PCMType* buffer,
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

    size_t const neonIterations = rest / STEREO_SAMPLES_PER_ITERATION;
    int32x4_t const nominator = NeonConverter::MakeNominator ( leftGain, rightGain );
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();

    PCMType const* neonSource = pcm;
    PCMType* neonTarget = buffer + bufferSamples - rest;

    for ( size_t i = 0U; i < neonIterations; ++i )
    {
        int32_t scratchPad[] =
        {
            neonSource[ 0U ], neonSource[ 1U ], neonSource[ 2U ], neonSource[ 3U ],
            neonSource[ 4U ], neonSource[ 5U ], neonSource[ 6U ], neonSource[ 7U ],
            neonSource[ 8U ], neonSource[ 9U ], neonSource[ 10U ], neonSource[ 11U ],
            neonSource[ 12U ], neonSource[ 13U ], neonSource[ 14U ], neonSource[ 15U ]
        };

        NeonConverter::Convert ( neonTarget, scratchPad, nominator, correction );
        neonSource += STEREO_SAMPLES_PER_ITERATION;
        neonTarget += STEREO_SAMPLES_PER_ITERATION;
    }

    for ( size_t i = neonIterations * STEREO_SAMPLES_PER_ITERATION; i < rest; i += 2U )
    {
        size_t const rightIdx = i + 1U;

        auto const leftSample = static_cast<int32_t> ( pcm[ i ] );
        auto const rightSample = static_cast<int32_t> ( pcm[ rightIdx ] );

        buffer[ i ] = static_cast<PCMType> ( leftSample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( rightSample * rightGain / DENOMINATOR );
    }

    _offset = rest;
}

void PCMStreamerWAV::HandleLoopedMono ( PCMType* buffer,
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

    size_t const restInPCMSamples = restInBufferSamples >> 1U;
    size_t const neonIterations = restInPCMSamples / MONO_SAMPLES_PER_ITERATION;
    int32x4_t const nominator = NeonConverter::MakeNominator ( leftGain, rightGain );
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();

    PCMType const* neonSource = pcm;
    PCMType* neonTarget = buffer + bufferSamples - restInBufferSamples;

    for ( size_t i = 0U; i < neonIterations; ++i )
    {
        int32_t scratchPad[] =
        {
            neonSource[ 0U ], neonSource[ 0U ], neonSource[ 1U ], neonSource[ 1U ],
            neonSource[ 2U ], neonSource[ 2U ], neonSource[ 3U ], neonSource[ 3U ],
            neonSource[ 4U ], neonSource[ 4U ], neonSource[ 5U ], neonSource[ 5U ],
            neonSource[ 6U ], neonSource[ 6U ], neonSource[ 7U ], neonSource[ 7U ]
        };

        NeonConverter::Convert ( neonTarget, scratchPad, nominator, correction );
        neonSource += MONO_SAMPLES_PER_ITERATION;
        neonTarget += STEREO_SAMPLES_PER_ITERATION;
    }

    for ( size_t i = neonIterations * MONO_SAMPLES_PER_ITERATION; i < restInPCMSamples; i += 2U )
    {
        size_t const leftIdx = i << 1U;
        size_t const rightIdx = leftIdx + 1U;

        auto const sample = static_cast<int32_t> ( pcm[ i ] );

        buffer[ leftIdx ] = static_cast<PCMType> ( sample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( sample * rightGain / DENOMINATOR );
    }

    _offset = restInPCMSamples;
}

PCMStreamerWAV::Consume PCMStreamerWAV::HandleMono ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const bufferFrames = bufferSamples >> 1U;

    // Buffer size should be smaller than total samples. Otherwise it's needed a more complicated implementation.
    assert ( bufferFrames <= _sampleCount );

    size_t const cases[] = { bufferFrames, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferFrames > _sampleCount ) ];

    size_t const neonIterations = canRead / MONO_SAMPLES_PER_ITERATION;
    int32x4_t const nominator = NeonConverter::MakeNominator ( leftGain, rightGain );
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();

    PCMType const* neonSource = pcm;
    PCMType* neonTarget = buffer;

    for ( size_t i = 0U; i < neonIterations; ++i )
    {
        int32_t scratchPad[] =
        {
            neonSource[ 0U ], neonSource[ 0U ], neonSource[ 1U ], neonSource[ 1U ],
            neonSource[ 2U ], neonSource[ 2U ], neonSource[ 3U ], neonSource[ 3U ],
            neonSource[ 4U ], neonSource[ 4U ], neonSource[ 5U ], neonSource[ 5U ],
            neonSource[ 6U ], neonSource[ 6U ], neonSource[ 7U ], neonSource[ 7U ]
        };

        NeonConverter::Convert ( neonTarget, scratchPad, nominator, correction );
        neonSource += MONO_SAMPLES_PER_ITERATION;
        neonTarget += STEREO_SAMPLES_PER_ITERATION;
    }

    for ( size_t i = neonIterations * MONO_SAMPLES_PER_ITERATION; i < canRead; i += 2U )
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
        ._pcmSampleCount = canRead
    };
}

PCMStreamerWAV::Consume PCMStreamerWAV::HandleStereo ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    // Buffer size should be smaller than total samples. Otherwise it's needed a more complicated implementation.
    assert ( _sampleCount >= bufferSamples );

    size_t const cases[] = { bufferSamples, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferSamples > _sampleCount ) ];

    size_t const neonIterations = canRead / STEREO_SAMPLES_PER_ITERATION;
    int32x4_t const nominator = NeonConverter::MakeNominator ( leftGain, rightGain );
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();

    PCMType const* neonSource = pcm;
    PCMType* neonTarget = buffer;

    for ( size_t i = 0U; i < neonIterations; ++i )
    {
        int32_t scratchPad[] =
        {
            neonSource[ 0U ], neonSource[ 1U ], neonSource[ 2U ], neonSource[ 3U ],
            neonSource[ 4U ], neonSource[ 5U ], neonSource[ 6U ], neonSource[ 7U ],
            neonSource[ 8U ], neonSource[ 9U ], neonSource[ 10U ], neonSource[ 11U ],
            neonSource[ 12U ], neonSource[ 13U ], neonSource[ 14U ], neonSource[ 15U ]
        };

        NeonConverter::Convert ( neonTarget, scratchPad, nominator, correction );
        neonSource += STEREO_SAMPLES_PER_ITERATION;
        neonTarget += STEREO_SAMPLES_PER_ITERATION;
    }

    for ( size_t i = neonIterations * STEREO_SAMPLES_PER_ITERATION; i < canRead; i += 2U )
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
        ._pcmSampleCount = canRead
    };
}

} // namespace android_vulkan
