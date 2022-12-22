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
        int32_t                 _bufferFrameCount = std::numeric_limits<int32_t>::min ();
        size_t                  _bufferSampleCount = std::numeric_limits<size_t>::max ();
        AAudioStreamBuilder*    _builder = nullptr;

    public:
        SoundMixer () = default;

        SoundMixer ( SoundMixer const & ) = delete;
        SoundMixer& operator = ( SoundMixer const & ) = delete;

        SoundMixer ( SoundMixer && ) = delete;
        SoundMixer& operator = ( SoundMixer && ) = delete;

        ~SoundMixer () = default;

        [[maybe_unused, nodiscard]] bool Init () noexcept;
        void Destroy () noexcept;

        // Note this is exact amount of samples for all channels in total.
        // For example stereo asset with 7 frames will return 14 samples.
        [[maybe_unused, nodiscard]] size_t GetBufferSampleCount () const noexcept;

        // Method returns true if "result" equals AAUDIO_OK. Otherwise method returns false.
        [[nodiscard]] static bool CheckAAudioResult ( aaudio_result_t result,
            char const* from,
            char const* message
        ) noexcept;

        [[nodiscard]] constexpr static int32_t GetChannelCount () noexcept
        {
            return 2;
        }

        [[nodiscard]] constexpr static aaudio_format_t GetFormat () noexcept
        {
            return AAUDIO_FORMAT_PCM_I16;
        }

        [[nodiscard]] constexpr static int32_t GetSampleRate () noexcept
        {
            return 44100;
        }

    private:
        [[nodiscard]] bool ResolveBufferSize () noexcept;

        static void PrintStreamInfo ( AAudioStream &stream, char const* header ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_MIXER_H
