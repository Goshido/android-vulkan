#ifndef ANDROID_VULKAN_SOUND_EMITTER_SPATIAL_H
#define ANDROID_VULKAN_SOUND_EMITTER_SPATIAL_H


#include "sound_emitter.h"
#include <GXCommon/GXMath.h>


namespace android_vulkan {

class [[maybe_unused]] SoundEmitterSpatial final : public SoundEmitter
{
    private:
        float       _distanceFactor = 1.0F;
        GXVec3      _location { 0.0F, 0.0F, 0.0F };

    public:
        SoundEmitterSpatial () = default;

        SoundEmitterSpatial ( SoundEmitterSpatial const & ) = delete;
        SoundEmitterSpatial& operator = ( SoundEmitterSpatial const & ) = delete;

        SoundEmitterSpatial ( SoundEmitterSpatial && ) = delete;
        SoundEmitterSpatial& operator = ( SoundEmitterSpatial && ) = delete;

        ~SoundEmitterSpatial () override = default;

        [[maybe_unused, nodiscard]] bool Init ( SoundMixer &soundMixer,
            eSoundChannel channel,
            GXVec3 const &location,
            float distance
        ) noexcept;

        void SetDistance ( float distance ) noexcept;
        void SetLocation ( GXVec3 const &location ) noexcept;

    private:
        void FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept override;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_EMITTER_SPATIAL_H
