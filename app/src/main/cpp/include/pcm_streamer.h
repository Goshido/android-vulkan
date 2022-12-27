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

        constexpr static int16_t INTEGER_DIVISION_SCALE = std::numeric_limits<int16_t>::max () - 1U;

    protected:
        struct Consume final
        {
            size_t                  _bufferSampleCount = 0U;
            bool                    _lastPCMBuffer = false;
            size_t                  _pcmSampleCount = 0U;
        };

        struct Info final
        {
            uint8_t                 _bytesPerChannelSample = 2U;
            uint8_t                 _channelCount = 1U;
            uint32_t                _sampleRate = 44100U;
        };

        using ReadHandler = Consume ( PCMStreamer::* ) ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            bool lastPCMBuffer,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        using LoopHandler = void ( PCMStreamer::* ) ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            Consume consume,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

    private:
        bool                        _decompressor = false;

    protected:
        OnStopRequest               _onStopRequest = nullptr;
        SoundEmitter&               _soundEmitter;
        SoundStorage::SoundFile     _soundFile {};

        size_t                      _offset = 0U;
        LoopHandler                 _loopHandler = &PCMStreamer::HandleLoopedMono;
        ReadHandler                 _readHandler = &PCMStreamer::HandleMono;
        size_t                      _sampleCount = 0U;

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

        virtual void OnDecompress () noexcept;

        // Method must be called by child classes,
        [[nodiscard]] virtual bool Reset () noexcept;

        [[nodiscard]] bool IsDecompressor () const noexcept;

        [[nodiscard]] bool SetSoundAsset ( SoundStorage &soundStorage,
            std::string_view const file,
            bool looped,
            size_t samplesPerBurst
        ) noexcept;

    protected:
        explicit PCMStreamer ( SoundEmitter &soundEmitter, OnStopRequest callback, bool decompressor ) noexcept;

        [[nodiscard]] virtual std::optional<Info> ResolveInfo ( bool looped, size_t samplesPerBurst ) noexcept = 0;

    private:
        void HandleLoopedMono ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            Consume consume,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        void HandleLoopedStereo ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            Consume consume,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        void HandleNonLooped ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            Consume consume,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        [[nodiscard]] Consume HandleMono ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            bool lastPCMBuffer,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        [[nodiscard]] Consume HandleStereo ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            bool lastPCMBuffer,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        [[nodiscard]] static bool IsFormatSupported ( std::string_view const file, Info const &info ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_H
