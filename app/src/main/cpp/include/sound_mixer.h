#ifndef ANDROID_VULKAN_SOUND_MIXER_H
#define ANDROID_VULKAN_SOUND_MIXER_H


#include "GXCommon/GXWarning.h"

GX_DISABLE_COMMON_WARNINGS

#include <limits>
#include <aaudio/AAudio.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class [[maybe_unused]] SoundMixer final
{
    private:
        AAudioStreamBuilder*        _buildler = nullptr;
        [[maybe_unused]] size_t     _burstSize = std::numeric_limits<size_t>::max ();

    public:
        SoundMixer () = default;

        SoundMixer ( SoundMixer const & ) = delete;
        SoundMixer& operator = ( SoundMixer const & ) = delete;

        SoundMixer ( SoundMixer && ) = delete;
        SoundMixer& operator = ( SoundMixer && ) = delete;

        ~SoundMixer () = default;

        [[maybe_unused, nodiscard]] bool Init () noexcept;
        void Destroy () noexcept;

        // Method returns true is "result" equals VK_SUCCESS. Otherwise method returns false.
        [[nodiscard]] static bool CheckAAudioResult ( aaudio_result_t result,
            char const* from,
            char const* message
        ) noexcept;

    private:
        [[maybe_unused]] void ResolveBurstSize () noexcept;

        [[nodiscard]] static char const* ResolveAAudioResult ( aaudio_result_t result ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_MIXER_H
