#ifndef ANDROID_VULKAN_SOUND_EMITTER_SPATIAL_HPP
#define ANDROID_VULKAN_SOUND_EMITTER_SPATIAL_HPP


#include "sound_emitter.hpp"
#include <GXCommon/GXMath.hpp>


namespace android_vulkan {

class SoundEmitterSpatial final : public SoundEmitter
{
    private:
        float       _distanceFactor = 1.0F;
        GXVec3      _location { 0.0F, 0.0F, 0.0F };
        GXVec2      _previousVolume { 1.0F, 1.0F };
        bool        _isNeedResetPreviousVolume = true;

    public:
        SoundEmitterSpatial () = default;

        SoundEmitterSpatial ( SoundEmitterSpatial const & ) = delete;
        SoundEmitterSpatial &operator = ( SoundEmitterSpatial const & ) = delete;

        SoundEmitterSpatial ( SoundEmitterSpatial && ) = delete;
        SoundEmitterSpatial &operator = ( SoundEmitterSpatial && ) = delete;

        ~SoundEmitterSpatial () override = default;

        [[nodiscard]] bool Play () noexcept override;
        void SetVolume ( float volume ) noexcept override;

        void SetDistance ( float distance ) noexcept;
        void SetLocation ( GXVec3 const &location ) noexcept;

    private:
        void FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept override;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_EMITTER_SPATIAL_HPP
