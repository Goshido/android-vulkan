#ifndef ANDROID_VULKAN_SOUND_EMITTER_H
#define ANDROID_VULKAN_SOUND_EMITTER_H


#include "pcm_streamer.h"
#include "sound_channel.h"

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
            eSoundChannel               _soundChannel = eSoundChannel::VFX;
            SoundMixer*                 _soundMixer = nullptr;
            SoundEmitter*               _soundEmitter = nullptr;
        };

    protected:
        enum class eStreamerType : uint16_t
        {
            UNKNOWN,
            OGG,
            WAV
        };

    protected:
        Context                         _context {};
        std::string                     _file {};
        bool                            _isPlaying = false;
        AAudioStream*                   _stream = nullptr;
        std::unique_ptr<PCMStreamer>    _streamer {};
        float                           _volume = 1.0F;

    public:
        SoundEmitter ( SoundEmitter const & ) = delete;
        SoundEmitter& operator = ( SoundEmitter const & ) = delete;

        SoundEmitter ( SoundEmitter && ) = delete;
        SoundEmitter& operator = ( SoundEmitter && ) = delete;

        virtual void FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept = 0;

        // Note method should be call in child class.
        virtual void SetVolume ( float volume ) noexcept;

        [[nodiscard]] bool Destroy () noexcept;

        [[nodiscard]] Context& GetContext () noexcept;
        [[nodiscard, maybe_unused]] std::string const& GetFile () const noexcept;
        [[nodiscard]] float GetVolume () const noexcept;

        [[nodiscard]] bool IsPlaying () const noexcept;
        [[nodiscard]] bool Pause () noexcept;
        [[nodiscard]] bool Play () noexcept;
        [[nodiscard]] bool Stop () noexcept;

        void OnStreamRecreated ( AAudioStream &stream ) noexcept;
        [[nodiscard]] bool SetSoundAsset ( std::string_view const file, bool looped ) noexcept;

    protected:
        SoundEmitter () = default;
        virtual ~SoundEmitter () = default;

        [[nodiscard]] bool InitInternal ( SoundMixer &soundMixer, eSoundChannel channel ) noexcept;

        [[nodiscard]] static eStreamerType GetStreamerType ( std::string_view const asset ) noexcept;
        static void OnStopRequested ( SoundEmitter &soundEmitter ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_EMITTER_H
