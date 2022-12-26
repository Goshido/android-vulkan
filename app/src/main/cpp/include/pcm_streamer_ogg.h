#ifndef ANDROID_VULKAN_PCM_STREAMER_OGG_H
#define ANDROID_VULKAN_PCM_STREAMER_OGG_H


#include "pcm_streamer.h"

GX_DISABLE_COMMON_WARNINGS

#include <vorbis/vorbisfile.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class PCMStreamerOGG final : public PCMStreamer
{
    private:
        struct Consume final
        {
            size_t                      _bufferSampleCount = 0U;
            size_t                      _pcmSampleCount = 0U;
        };

        using LoopHandler = void ( PCMStreamerOGG::* ) ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            Consume consume,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        using ReadHandler = Consume ( PCMStreamerOGG::* ) ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        struct DecompressData final
        {
            size_t                      _sampleCount = 0U;
            std::vector<PCMType>        _samples {};
        };

        constexpr static size_t         BUFFER_COUNT = 2U;

    private:
        size_t                          _activeBufferIndex = 0U;
        DecompressData                  _decompressedBuffers[ BUFFER_COUNT ]{};
        uint8_t                         _readyBuffers = 0U;

        OggVorbis_File                  _fileOGG {};
        bool                            _isFileOpen = false;
        bool                            _looped = false;

        size_t                          _offset = 0U;
        LoopHandler                     _loopHandler = &PCMStreamerOGG::HandleLoopedMono;

        ogg_int64_t                     _positionOGG = 0;

        ReadHandler                     _readHandler = &PCMStreamerOGG::HandleMono;
        size_t                          _sampleCount = 0U;

    public:
        PCMStreamerOGG () = delete;

        PCMStreamerOGG ( PCMStreamerOGG const & ) = delete;
        PCMStreamerOGG& operator = ( PCMStreamerOGG const & ) = delete;

        PCMStreamerOGG ( PCMStreamerOGG && ) = delete;
        PCMStreamerOGG& operator = ( PCMStreamerOGG && ) = delete;

        explicit PCMStreamerOGG ( SoundEmitter &soundEmitter, OnStopRequest callback ) noexcept;

        ~PCMStreamerOGG () override;

        // Method returns true if "result" equals 0. Otherwise method returns false.
        static bool CheckVorbisResult ( int result, char const* from, char const* message ) noexcept;

    private:
        void GetNextBuffer ( std::span<PCMType> buffer,
            float leftChannelVolume,
            float rightChannelVolume
        ) noexcept override;

        void OnDecompress () noexcept override;
        [[nodiscard]] std::optional<Info> ResolveInfo ( bool looped, size_t samplesPerBurst ) noexcept override;

        void AllocateDecompressBuffers ( size_t channels, size_t samplesPerBurst ) noexcept;

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
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        [[nodiscard]] Consume HandleStereo ( PCMType* buffer,
            size_t bufferSamples,
            PCMType const* pcm,
            int32_t leftGain,
            int32_t rightGain
        ) noexcept;

        // ogg I/O:
        [[nodiscard]] int CloseInternal () noexcept;
        [[nodiscard]] size_t ReadInternal ( void* ptr, size_t size, size_t count) noexcept;
        [[nodiscard]] int SeekInternal (ogg_int64_t offset, int whence ) noexcept;
        [[nodiscard]] long TellInternal () const noexcept;

        [[nodiscard]] static int Close ( void* datasource );
        [[nodiscard]] static size_t Read ( void* ptr, size_t size, size_t count, void* datasource );
        [[nodiscard]] static int Seek ( void* datasource, ogg_int64_t offset, int whence );
        [[nodiscard]] static long Tell ( void* datasource );
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_OGG_H
