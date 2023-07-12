#include <av_assert.h>
#include <logger.h>
#include <sound_emitter_spatial.h>
#include <sound_mixer.h>


namespace android_vulkan {

void SoundEmitterSpatial::SetVolume ( float volume ) noexcept
{
    SoundEmitter::SetVolume ( volume );
    _previousVolume = GXVec2 ( volume, volume );
}

void SoundEmitterSpatial::SetDistance ( float distance ) noexcept
{
    AV_ASSERT ( distance > 0.0F )
    _distanceFactor = 1.0F / distance;
}

void SoundEmitterSpatial::SetLocation ( GXVec3 const &location ) noexcept
{
    _location = location;
}

void SoundEmitterSpatial::FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept
{
    AV_ASSERT ( _streamer )

    SoundListenerInfo const &listener = _mixer->GetListenerInfo ();
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

    GXVec2 volume ( soundDirection.DotProduct ( listener._leftDirection ),
        soundDirection.DotProduct ( listener._rightDirection )
    );

    float const attenuation = channelVolume * _volume * distanceAttenuation;
    constexpr GXVec2 factor ( 0.5F, 0.5F );
    volume.Sum ( factor, 0.5F, volume );
    volume.Multiply ( volume, attenuation );

    _streamer->GetNextBuffer ( buffer,
        PCMStreamer::Gain ( _previousVolume._data[ 0U ], volume._data[ 0U ] ),
        PCMStreamer::Gain ( _previousVolume._data[ 1U ], volume._data[ 1U ] )
    );

    _previousVolume = volume;
}

} // namespace android_vulkan
