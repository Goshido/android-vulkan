#include <logger.h>
#include <sound_emitter_spatial.h>
#include <sound_mixer.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

[[maybe_unused]] bool SoundEmitterSpatial::Init ( SoundMixer &soundMixer,
    eSoundChannel channel,
    GXVec3 const &location,
    float distance
) noexcept
{
    SetLocation ( location );
    SetDistance ( distance );
    return InitInternal ( soundMixer, channel );
}

void SoundEmitterSpatial::SetDistance ( float distance ) noexcept
{
    assert ( distance > 0.0F );
    _distanceFactor = 1.0F / distance;
}

void SoundEmitterSpatial::SetLocation ( GXVec3 const &location ) noexcept
{
    _location = location;
}

void SoundEmitterSpatial::FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept
{
    assert ( _streamer );

    SoundListenerInfo const& listener = _context._soundMixer->GetListenerInfo ();
    GXVec3 soundDirection {};
    soundDirection.Subtract ( _location, listener._location );

    float const distanceAttenuation = std::max ( 0.0F,
        -1.0F + 2.0F / ( 1.0F + std::sqrt ( _distanceFactor * soundDirection.Length () ) )
    );

    soundDirection.Normalize ();

    if ( std::isnan ( soundDirection._data[ 0U ] ) )
    {
        // Sound source is located directly in sound listener. Rare but possible.
        constexpr GXVec3 fallbackDirection ( 0.0F, 0.0F, 1.0F );
        soundDirection = fallbackDirection;
    }

    GXVec2 leftRight ( soundDirection.DotProduct ( listener._leftDirection ),
        soundDirection.DotProduct ( listener._rightDirection )
    );

    float const attenuation = channelVolume * _volume * distanceAttenuation;
    constexpr GXVec2 factor ( 0.5F, 0.5F );
    leftRight.Sum ( factor, 0.5F, leftRight );
    leftRight.Multiply ( leftRight, attenuation );
    _streamer->GetNextBuffer ( buffer, leftRight._data[ 0U ], leftRight._data[ 1U ] );
}

} // namespace android_vulkan
