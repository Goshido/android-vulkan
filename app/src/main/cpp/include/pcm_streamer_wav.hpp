#ifndef ANDROID_VULKAN_PCM_STREAMER_WAV_HPP
#define ANDROID_VULKAN_PCM_STREAMER_WAV_HPP


#include "pcm_streamer.hpp"


namespace android_vulkan {

class PCMStreamerWAV final : public PCMStreamer
{
    public:
        PCMStreamerWAV () = delete;

        PCMStreamerWAV ( PCMStreamerWAV const & ) = delete;
        PCMStreamerWAV &operator = ( PCMStreamerWAV const & ) = delete;

        PCMStreamerWAV ( PCMStreamerWAV && ) = delete;
        PCMStreamerWAV &operator = ( PCMStreamerWAV && ) = delete;

        explicit PCMStreamerWAV ( SoundEmitter &soundEmitter, OnStopRequest callback ) noexcept;

        ~PCMStreamerWAV () override = default;

    private:
        void GetNextBuffer ( std::span<PCMType> buffer, Gain left, Gain right ) noexcept override;
        [[nodiscard]] std::optional<Info> ResolveInfo ( bool looped, size_t samplesPerBurst ) noexcept override;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_WAV_HPP
