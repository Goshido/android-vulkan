#ifndef ANDROID_VULKAN_PCM_STREAMER_H
#define ANDROID_VULKAN_PCM_STREAMER_H


#include "sound_storage.h"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class SoundEmitter;

class PCMStreamer
{
    public:
        using PCMType = int16_t;
        using OnStopRequest = void ( * ) ( SoundEmitter &emitter ) noexcept;

    protected:
        struct Info final
        {
            uint8_t                 _bytesPerChannelSample = 2U;
            uint8_t                 _channelCount = 1U;
            uint32_t                _sampleRate = 44100U;
        };

    protected:
        OnStopRequest               _onStopRequest = nullptr;
        SoundEmitter&               _soundEmitter;
        SoundStorage::SoundFile     _soundFile {};

    public:
        PCMStreamer () = delete;

        PCMStreamer ( PCMStreamer const & ) = delete;
        PCMStreamer& operator = ( PCMStreamer const & ) = delete;

        PCMStreamer ( PCMStreamer && ) = delete;
        PCMStreamer& operator = ( PCMStreamer && ) = delete;

        virtual ~PCMStreamer () = default;

        virtual void GetNextBuffer ( std::span<PCMType> buffer, float leftChannelVolume,
            float rightChannelVolume
        ) noexcept = 0;

        [[nodiscard]] bool SetSoundAsset ( SoundStorage &soundStorage,
            std::string_view const file,
            bool looped
        ) noexcept;

    protected:
        explicit PCMStreamer ( SoundEmitter &soundEmitter, OnStopRequest callback ) noexcept;

        [[nodiscard]] virtual std::optional<Info> ResolveInfo ( bool looped ) noexcept = 0;

        [[nodiscard]] static bool IsFormatSupported ( std::string_view const file, Info const &info ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_H
