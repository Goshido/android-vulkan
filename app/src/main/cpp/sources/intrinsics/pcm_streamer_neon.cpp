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
constexpr size_t STEREO_SAMPLES_PER_BATCH = IN_NEON_REGISTERS * SAMPLES_PER_REGISTER;
constexpr size_t MONO_SAMPLES_PER_BATCH = STEREO_SAMPLES_PER_BATCH / 2U;

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

        static void Convert ( PCMStreamer::PCMType* target,
            int32x4_t const &before,
            int32x4_t const &diff,
            int32x4_t const &correction,
            int16_t leftSample,
            int16_t rightSample,
            int32_t sampleFactor,
            int32_t sampleCount
        ) noexcept;

        [[nodiscard]] static int32x4_t MakeBeforeFactor ( int32_t leftBefore, int32_t rightBefore ) noexcept;
        [[nodiscard]] static int32x4_t MakeCorrectionVector () noexcept;

        [[nodiscard]] static int32x4_t MakeDiffFactor ( int32_t currentLeft,
            int32_t currentRight,
            int32x4_t const &before
        ) noexcept;

        [[nodiscard]] static int32x4_t MakeNominator ( int32x4_t const &before,
            int32x4_t const &diff,
            int32x4_t const &correction,
            int32_t sampleFactor,
            int32_t sampleCount
        ) noexcept;

        static void UpdateGain ( int32_t &left,
            int32_t &right,
            int32x4_t const &before,
            int32x4_t const &diff,
            int32x4_t const &correction,
            int32_t sampleFactor,
            int32_t sampleCount
        ) noexcept;

    private:
        [[nodiscard]] static int32x2_t Divide ( int32x2_t v, int32x4_t const &correction ) noexcept;
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

    for ( size_t idx = 0U; idx < STEREO_SAMPLES_PER_BATCH; ++idx )
    {
        target[ idx ] = static_cast<PCMStreamer::PCMType>( scratchPad[ idx ] );
    }
}

void NeonConverter::Convert ( PCMStreamer::PCMType* target,
    int32x4_t const &before,
    int32x4_t const &diff,
    int32x4_t const &correction,
    int16_t leftSample,
    int16_t rightSample,
    int32_t sampleFactor,
    int32_t sampleCount
) noexcept
{
    int32_t data[] = { leftSample, rightSample };
    int32x2_t const s = vld1_s32 ( data );

    int32x2_t const alpha = vmul_s32 ( vget_high_s32 ( diff ), vdup_n_s32 ( sampleFactor / sampleCount ) );
    int32x2_t const n = vadd_s32 ( vget_high_s32 ( before ), Divide ( alpha, correction ) );
    
    vst1_s32 ( data, NeonConverter::Divide ( vmul_s32 ( s, n ), correction ) );
    target[ 0U ] = data[ 0U ];
    target[ 1U ] = data[ 1U ];
}

int32x4_t NeonConverter::MakeBeforeFactor ( int32_t leftBefore, int32_t rightBefore ) noexcept
{
    int32_t const data[] = { leftBefore, rightBefore, leftBefore, rightBefore };
    return vld1q_s32 ( data );
}

int32x4_t NeonConverter::MakeCorrectionVector () noexcept
{
    return vdupq_n_s32 ( DENOMINATOR - 1 );
}

int32x4_t NeonConverter::MakeDiffFactor ( int32_t currentLeft,
    int32_t currentRight,
    int32x4_t const &before
) noexcept
{
    int32_t const data[] = { currentLeft, currentRight, currentLeft, currentRight };
    return vsubq_s32 ( vld1q_s32 ( data ), before );
}

int32x4_t NeonConverter::MakeNominator ( int32x4_t const &beforeFactor,
    int32x4_t const &diffFactor,
    int32x4_t const &correction,
    int32_t sampleFactor,
    int32_t sampleCount
) noexcept
{
    int32x4_t const alpha = vmulq_s32 ( diffFactor, vdupq_n_s32 ( sampleFactor / sampleCount ) );
    return vaddq_s32 ( beforeFactor, Divide ( alpha, correction ) );
}

void NeonConverter::UpdateGain ( int32_t &left,
    int32_t &right,
    int32x4_t const &before,
    int32x4_t const &diff,
    int32x4_t const &correction,
    int32_t sampleFactor,
    int32_t sampleCount
) noexcept
{
    int32x2_t const alpha = vmul_s32 ( vget_high_s32 ( diff ), vdup_n_s32 ( sampleFactor / sampleCount ) );

    int32_t gains[ 2U ];
    vst1_s32 ( gains, vadd_s32 ( vget_high_s32 ( before ), Divide ( alpha, correction ) ) );

    left = gains[ 0U ];
    right = gains[ 1U ];
}

int32x2_t NeonConverter::Divide ( int32x2_t v, int32x4_t const &correction ) noexcept
{
    return vshr_n_s32 ( vadd_s32 ( v, vand_s32 ( vcltz_s32 ( v ), vget_high_s32 ( correction ) ) ), 15 );
}

int32x4_t NeonConverter::Divide ( int32x4_t v, int32x4_t const &correction ) noexcept
{
    return vshrq_n_s32 ( vaddq_s32 ( v, vandq_s32 ( vcltzq_s32 ( v ), correction ) ), 15 );
}

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

    size_t const restInPCMSamples = restInBufferSamples >> 1U;
    auto const restSamples = static_cast<int32_t> ( restInPCMSamples );
    auto const batches = static_cast<int32_t> ( restInPCMSamples / MONO_SAMPLES_PER_BATCH );

    constexpr int32_t aFactor = MONO_SAMPLES_PER_BATCH * DENOMINATOR;
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();
    int32x4_t const before = NeonConverter::MakeBeforeFactor ( leftGain._before, rightGain._before );
    int32x4_t const diff = NeonConverter::MakeDiffFactor ( leftGain._current, rightGain._current, before );

    PCMType const* source = pcm;
    PCMType* target = buffer.data () + ( bufferSamples - restInBufferSamples );

    for ( int32_t i = 0; i < batches; ++i )
    {
        int32x4_t const nominator = NeonConverter::MakeNominator ( before, diff, correction, i * aFactor, restSamples );

        int32_t scratchPad[] =
        {
            source[ 0U ], source[ 0U ], source[ 1U ], source[ 1U ],
            source[ 2U ], source[ 2U ], source[ 3U ], source[ 3U ],
            source[ 4U ], source[ 4U ], source[ 5U ], source[ 5U ],
            source[ 6U ], source[ 6U ], source[ 7U ], source[ 7U ]
        };

        NeonConverter::Convert ( target, scratchPad, nominator, correction );
        source += MONO_SAMPLES_PER_BATCH;
        target += STEREO_SAMPLES_PER_BATCH;
    }

    for ( int32_t i = batches * static_cast<int32_t> ( MONO_SAMPLES_PER_BATCH ); i < restSamples; ++i )
    {
        int16_t const sample = pcm[ i ];
        NeonConverter::Convert ( target, before, diff, correction, sample, sample, i * DENOMINATOR, restSamples );
        target += 2U;
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

    auto const restSamples = static_cast<int32_t> ( rest );
    auto const batches = static_cast<int32_t> ( rest / STEREO_SAMPLES_PER_BATCH );

    constexpr int32_t aFactor = STEREO_SAMPLES_PER_BATCH * DENOMINATOR;
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();
    int32x4_t const before = NeonConverter::MakeBeforeFactor ( leftGain._before, rightGain._before );
    int32x4_t const diff = NeonConverter::MakeDiffFactor ( leftGain._current, rightGain._current, before );

    PCMType const* source = pcm;
    PCMType* target = buffer.data () + ( bufferSamples - rest );

    for ( int32_t i = 0; i < batches; ++i )
    {
        int32x4_t const nominator = NeonConverter::MakeNominator ( before, diff, correction, i * aFactor, restSamples );

        int32_t scratchPad[] =
        {
            source[ 0U ], source[ 1U ], source[ 2U ], source[ 3U ],
            source[ 4U ], source[ 5U ], source[ 6U ], source[ 7U ],
            source[ 8U ], source[ 9U ], source[ 10U ], source[ 11U ],
            source[ 12U ], source[ 13U ], source[ 14U ], source[ 15U ]
        };

        NeonConverter::Convert ( target, scratchPad, nominator, correction );
        source += STEREO_SAMPLES_PER_BATCH;
        target += STEREO_SAMPLES_PER_BATCH;
    }

    for ( int32_t i = batches * static_cast<int32_t> ( STEREO_SAMPLES_PER_BATCH ); i < restSamples; i += 2 )
    {
        NeonConverter::Convert ( target,
            before,
            diff,
            correction,
            pcm[ i ],
            pcm[ i + 1 ],
            i * DENOMINATOR,
            restSamples
        );

        target += 2U;
    }

    _offset = rest;
}

PCMStreamer::Consume PCMStreamer::HandleMono ( std::span<PCMType> buffer,
    Gain &leftGain,
    Gain &rightGain,
    PCMType const* pcm,
    bool lastPCMBuffer
) noexcept
{
    size_t const bufferFrames = buffer.size () >> 1U;
    size_t const cases[] = { bufferFrames, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferFrames > _sampleCount ) ];

    auto const available = static_cast<int32_t> ( canRead );
    auto const batches = static_cast<int32_t> ( available / MONO_SAMPLES_PER_BATCH );
    auto const s = static_cast<int32_t> ( bufferFrames );

    constexpr int32_t aFactor = MONO_SAMPLES_PER_BATCH * DENOMINATOR;
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();
    int32x4_t const before = NeonConverter::MakeBeforeFactor ( leftGain._before, rightGain._before );
    int32x4_t const diff = NeonConverter::MakeDiffFactor ( leftGain._current, rightGain._current, before );

    PCMType const* source = pcm;
    PCMType* target = buffer.data ();

    for ( int32_t i = 0; i < batches; ++i )
    {
        int32x4_t const nominator = NeonConverter::MakeNominator ( before, diff, correction, i * aFactor, s );

        int32_t scratchPad[] =
        {
            source[ 0U ], source[ 0U ], source[ 1U ], source[ 1U ],
            source[ 2U ], source[ 2U ], source[ 3U ], source[ 3U ],
            source[ 4U ], source[ 4U ], source[ 5U ], source[ 5U ],
            source[ 6U ], source[ 6U ], source[ 7U ], source[ 7U ]
        };

        NeonConverter::Convert ( target, scratchPad, nominator, correction );
        source += MONO_SAMPLES_PER_BATCH;
        target += STEREO_SAMPLES_PER_BATCH;
    }

    for ( int32_t i = batches * static_cast<int32_t> ( MONO_SAMPLES_PER_BATCH ); i < available; ++i )
    {
        int16_t const sample = pcm[ i ];
        NeonConverter::Convert ( target, before, diff, correction, sample, sample, i * DENOMINATOR, s );
        target += 2U;
    }

    NeonConverter::UpdateGain ( leftGain._current, rightGain._current, before, diff, correction, available, s );

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
) noexcept
{
    size_t const bufferSamples = buffer.size ();
    size_t const cases[] = { bufferSamples, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferSamples > _sampleCount ) ];

    auto const available = static_cast<int32_t> ( canRead );
    auto const batches = static_cast<int32_t> ( available / STEREO_SAMPLES_PER_BATCH );
    auto const s = static_cast<int32_t> ( bufferSamples );

    constexpr int32_t aFactor = STEREO_SAMPLES_PER_BATCH * DENOMINATOR;
    int32x4_t const correction = NeonConverter::MakeCorrectionVector ();
    int32x4_t const before = NeonConverter::MakeBeforeFactor ( leftGain._before, rightGain._before );
    int32x4_t const diff = NeonConverter::MakeDiffFactor ( leftGain._current, rightGain._current, before );

    PCMType const* source = pcm;
    PCMType* target = buffer.data ();

    for ( int32_t i = 0; i < batches; ++i )
    {
        int32x4_t const nominator = NeonConverter::MakeNominator ( before, diff, correction, i * aFactor, s );

        int32_t scratchPad[] =
        {
            source[ 0U ], source[ 1U ], source[ 2U ], source[ 3U ],
            source[ 4U ], source[ 5U ], source[ 6U ], source[ 7U ],
            source[ 8U ], source[ 9U ], source[ 10U ], source[ 11U ],
            source[ 12U ], source[ 13U ], source[ 14U ], source[ 15U ]
        };

        NeonConverter::Convert ( target, scratchPad, nominator, correction );
        source += STEREO_SAMPLES_PER_BATCH;
        target += STEREO_SAMPLES_PER_BATCH;
    }

    for ( int32_t i = batches * static_cast<int32_t> ( STEREO_SAMPLES_PER_BATCH ); i < available; i += 2 )
    {
        NeonConverter::Convert ( target, before, diff, correction, pcm[ i ], pcm[ i + 1 ], i * DENOMINATOR, s );
        target += 2U;
    }

    NeonConverter::UpdateGain ( leftGain._current, rightGain._current, before, diff, correction, available, s );

    return Consume
    {
        ._bufferSampleCount = canRead,
        ._lastPCMBuffer = lastPCMBuffer,
        ._pcmSampleCount = canRead
    };
}

} // namespace android_vulkan
