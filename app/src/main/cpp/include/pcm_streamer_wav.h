#ifndef ANDROID_VULKAN_PCM_STREAMER_WAV_H
#define ANDROID_VULKAN_PCM_STREAMER_WAV_H


#include "pcm_streamer.h"


namespace android_vulkan {

class PCMStreamerWAV final : public PCMStreamer
{
    private:
        using LoopHandler = void ( PCMStreamerWAV::* ) ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            size_t canRead,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        using ReadHandler = void ( PCMStreamerWAV::* ) ( PCMType* buffer,
            size_t canRead,
            PCMType const* pcm,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

    private:
        size_t          _offset = 0U;
        LoopHandler     _loopHandler = &PCMStreamerWAV::HandleLoopedMono;
        ReadHandler     _readHandler = &PCMStreamerWAV::HandleMono;
        size_t          _sampleCount = 0U;

    public:
        PCMStreamerWAV () = delete;

        PCMStreamerWAV ( PCMStreamerWAV const & ) = delete;
        PCMStreamerWAV& operator = ( PCMStreamerWAV const & ) = delete;

        PCMStreamerWAV ( PCMStreamerWAV && ) = delete;
        PCMStreamerWAV& operator = ( PCMStreamerWAV && ) = delete;

        explicit PCMStreamerWAV ( SoundEmitter &soundEmitter, OnStopRequest callback ) noexcept;

        ~PCMStreamerWAV () override = default;

    private:
        void GetNextBuffer ( std::span<PCMType> buffer,
            float leftChannelVolume,
            float rightChannelVolume
        ) noexcept override;

        [[nodiscard]] std::optional<Info> ResolveInfo ( bool looped ) noexcept override;

        void HandleLoopedMono ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            size_t canRead,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        void HandleLoopedStereo ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            size_t canRead,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        void HandleNonLooped ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            size_t canRead,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        void HandleMono ( PCMType* buffer,
            size_t canRead,
            PCMType const* pcm,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        void HandleStereo ( PCMType* buffer,
            size_t canRead,
            PCMType const* pcm,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_WAV_H
