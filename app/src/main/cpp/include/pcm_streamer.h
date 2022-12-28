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
        constexpr static int16_t INTEGER_DIVISION_SCALE = std::numeric_limits<int16_t>::max () - 1U;

        using PCMType = int16_t;
        using OnStopRequest = void ( * ) ( SoundEmitter &emitter ) noexcept;

        class Gain final
        {
            public:
                int32_t             _before = static_cast<int32_t> ( INTEGER_DIVISION_SCALE );
                int32_t             _current = static_cast<int32_t> ( INTEGER_DIVISION_SCALE );

            public:
                Gain () = delete;

                Gain ( Gain const & ) = default;
                Gain& operator = ( Gain const & ) = default;

                Gain ( Gain && ) = default;
                Gain& operator = ( Gain && ) = default;

                constexpr explicit Gain ( float volumeBefore, float volumeCurrent ) noexcept:
                    _before ( static_cast<int32_t> ( volumeBefore * static_cast<float> ( INTEGER_DIVISION_SCALE ) ) ),
                    _current ( static_cast<int32_t> ( volumeCurrent * static_cast<float> ( INTEGER_DIVISION_SCALE ) ) )
                {
                    // NOTHING
                }

                ~Gain () = default;
        };

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

        using ReadHandler = Consume ( PCMStreamer::* ) ( std::span<PCMType> buffer,
            Gain &leftGain,
            Gain &rightGain,
            PCMType const* pcm,
            bool lastPCMBuffer
        ) noexcept;

        using LoopHandler = void ( PCMStreamer::* ) ( std::span<PCMType> buffer,
            PCMType const* pcm,
            Consume consume,
            Gain leftGain,
            Gain rightGain
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

        virtual void GetNextBuffer ( std::span<PCMType> buffer, Gain left, Gain right ) noexcept = 0;
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
        void HandleLoopedMono ( std::span<PCMType> buffer,
            PCMType const* pcm,
            Consume consume,
            Gain leftGain,
            Gain rightGain
        ) noexcept;

        void HandleLoopedStereo ( std::span<PCMType> buffer,
            PCMType const* pcm,
            Consume consume,
            Gain leftGain,
            Gain rightGain
        ) noexcept;

        void HandleNonLooped ( std::span<PCMType> buffer,
            PCMType const* pcm,
            Consume consume,
            Gain leftGain,
            Gain rightGain
        ) noexcept;

        [[nodiscard]] Consume HandleMono ( std::span<PCMType> buffer,
            Gain &leftGain,
            Gain &rightGain,
            PCMType const* pcm,
            bool lastPCMBuffer
        ) noexcept;

        [[nodiscard]] Consume HandleStereo ( std::span<PCMType> buffer,
            Gain &leftGain,
            Gain &rightGain,
            PCMType const* pcm,
            bool lastPCMBuffer
        ) noexcept;

        [[nodiscard]] static bool IsFormatSupported ( std::string_view const file, Info const &info ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_H
