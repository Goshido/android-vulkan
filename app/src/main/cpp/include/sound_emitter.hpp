#ifndef ANDROID_VULKAN_SOUND_EMITTER_HPP
#define ANDROID_VULKAN_SOUND_EMITTER_HPP


#include "pcm_streamer.hpp"
#include <android_vulkan_sdk/primitive_types.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <aaudio/AAudio.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class SoundMixer;

class SoundEmitter
{
    protected:
        enum class eStreamerType : uint16_t
        {
            UNKNOWN,
            OGG,
            WAV
        };

    protected:
        SoundMixer*                     _mixer = nullptr;
        std::unique_ptr<PCMStreamer>    _streamer {};
        float                           _volume = 1.0F;

    private:
        eSoundChannel                   _channel = eSoundChannel::SFX;
        std::string                     _file {};
        bool                            _hasStream = false;
        bool                            _isPlaying = false;

    public:
        SoundEmitter ( SoundEmitter const & ) = delete;
        SoundEmitter &operator = ( SoundEmitter const & ) = delete;

        SoundEmitter ( SoundEmitter && ) = delete;
        SoundEmitter &operator = ( SoundEmitter && ) = delete;

        virtual void FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept = 0;

        // Note method should be called in child class.
        [[nodiscard]] virtual bool Play () noexcept;

        // Note method should be called in child class.
        virtual void SetVolume ( float volume ) noexcept;

        void Init ( SoundMixer &soundMixer, eSoundChannel channel ) noexcept;
        [[nodiscard]] bool Destroy () noexcept;

        [[nodiscard]] std::string const &GetFile () const noexcept;
        [[nodiscard]] eSoundChannel GetSoundChannel () const noexcept;
        [[nodiscard]] float GetVolume () const noexcept;

        [[nodiscard]] bool IsPlaying () const noexcept;
        [[nodiscard]] bool Pause () noexcept;
        [[nodiscard]] bool Stop () noexcept;

        [[nodiscard]] bool SetSoundAsset ( std::string_view const file, bool looped ) noexcept;

    protected:
        SoundEmitter () = default;
        virtual ~SoundEmitter () = default;

        [[nodiscard]] static eStreamerType GetStreamerType ( std::string_view const asset ) noexcept;
        static void OnStopRequested ( SoundEmitter &soundEmitter ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_EMITTER_HPP
