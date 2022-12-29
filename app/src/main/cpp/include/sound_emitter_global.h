#ifndef ANDROID_VULKAN_SOUND_EMITTER_GLOBAL_H
#define ANDROID_VULKAN_SOUND_EMITTER_GLOBAL_H


#include "sound_emitter.h"


namespace android_vulkan {

class SoundEmitterGlobal final : public SoundEmitter
{
    public:
        SoundEmitterGlobal () = default;

        SoundEmitterGlobal ( SoundEmitterGlobal const & ) = delete;
        SoundEmitterGlobal& operator = ( SoundEmitterGlobal const & ) = delete;

        SoundEmitterGlobal ( SoundEmitterGlobal && ) = delete;
        SoundEmitterGlobal& operator = ( SoundEmitterGlobal && ) = delete;

        ~SoundEmitterGlobal () override = default;

        [[nodiscard]] bool Init ( SoundMixer &soundMixer, eSoundChannel channel ) noexcept;

    private:
        void FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept override;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_EMITTER_GLOBAL_H
