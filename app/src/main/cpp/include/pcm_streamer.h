#ifndef ANDROID_VULKAN_PCM_STREAMER_H
#define ANDROID_VULKAN_PCM_STREAMER_H


#include "sound_storage.h"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class PCMStreamer
{
    public:
        using PCMType = int16_t;

    protected:
        static constexpr size_t                     BUFFER_COUNT = 2U;

        struct Info final
        {
            [[maybe_unused]] uint8_t                _bytesPerChannelSample = 2U;
            uint8_t                                 _channelCount = 1U;
            uint32_t                                _sampleRate = 44100U;
        };

    protected:
        [[maybe_unused]] size_t                     _activeBuffer = 0U;
        std::vector<PCMType>                        _buffers[ BUFFER_COUNT ]{};
        uint8_t                                     _channelCount = 1U;
        [[maybe_unused]] SoundStorage::SoundFile    _soundFile {};

    public:
        PCMStreamer ( PCMStreamer const & ) = delete;
        PCMStreamer& operator = ( PCMStreamer const & ) = delete;

        PCMStreamer ( PCMStreamer && ) = delete;
        PCMStreamer& operator = ( PCMStreamer && ) = delete;

        virtual ~PCMStreamer () = default;

        [[maybe_unused, nodiscard]] virtual std::span<PCMType const> GetNextBuffer () noexcept = 0;

        [[maybe_unused, nodiscard]] uint8_t GetChannelCount () const noexcept;

        // Note bufferLengthMs is in milliseconds,
        [[maybe_unused, nodiscard]] bool SetSoundAsset ( SoundStorage &soundStorage,
            std::string &&file,
            size_t bufferLengthMs
        ) noexcept;

    protected:
        PCMStreamer () = default;

        [[nodiscard]] virtual std::optional<Info> ResolveInfo () noexcept = 0;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_H
