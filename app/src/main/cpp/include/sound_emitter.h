#ifndef ANDROID_VULKAN_SOUND_EMITTER_H
#define ANDROID_VULKAN_SOUND_EMITTER_H


#include "pcm_streamer.h"
#include "sound_channel.h"
#include "pcm_streamer.h"

GX_DISABLE_COMMON_WARNINGS

#include <aaudio/AAudio.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class SoundMixer;

class SoundEmitter
{
    public:
        struct Context final
        {
            SoundMixer*                     _soundMixer = nullptr;
            SoundEmitter*                   _soundEmitter = nullptr;
        };

    protected:
        enum class eStreamerType : uint16_t
        {
            OGG,
            WAV
        };

    protected:
        [[maybe_unused]] eSoundChannel      _channel = eSoundChannel::Music;
        Context                             _context {};
        AAudioStream*                       _stream = nullptr;
        std::unique_ptr<PCMStreamer>        _streamer {};

    public:
        SoundEmitter ( SoundEmitter const & ) = delete;
        SoundEmitter& operator = ( SoundEmitter const & ) = delete;

        SoundEmitter ( SoundEmitter && ) = delete;
        SoundEmitter& operator = ( SoundEmitter && ) = delete;

        virtual void FillPCM ( std::span<PCMStreamer::PCMType> buffer ) noexcept = 0;

        [[maybe_unused, nodiscard]] bool Init ( SoundMixer &soundMixer, eSoundChannel channel ) noexcept;
        [[maybe_unused, nodiscard]] bool Destroy () noexcept;

        [[maybe_unused, nodiscard]] bool SetSoundAsset ( SoundStorage &soundStorage,
            std::string_view const file
        ) noexcept;

    protected:
        SoundEmitter () = default;
        virtual ~SoundEmitter () = default;

        [[nodiscard]] static eStreamerType GetStreamerType ( std::string_view const asset ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_EMITTER_H
