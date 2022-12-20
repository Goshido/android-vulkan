#ifndef ANDROID_VULKAN_PCM_STREAMER_WAV_H
#define ANDROID_VULKAN_PCM_STREAMER_WAV_H


#include "pcm_streamer.h"


namespace android_vulkan {

class [[maybe_unused]] PCMStreamerWAV final : public PCMStreamer
{
    public:
        PCMStreamerWAV () = default;

        PCMStreamerWAV ( PCMStreamerWAV const & ) = delete;
        PCMStreamerWAV& operator = ( PCMStreamerWAV const & ) = delete;

        PCMStreamerWAV ( PCMStreamerWAV && ) = delete;
        PCMStreamerWAV& operator = ( PCMStreamerWAV && ) = delete;

        ~PCMStreamerWAV () override = default;

    private:
        [[maybe_unused, nodiscard]] std::span<PCMType const> GetNextBuffer () noexcept override;
        [[nodiscard]] std::optional<Info> ResolveInfo () noexcept override;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_PCM_STREAMER_WAV_H
