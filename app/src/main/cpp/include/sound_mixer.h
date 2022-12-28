#ifndef ANDROID_VULKAN_SOUND_MIXER_H
#define ANDROID_VULKAN_SOUND_MIXER_H


#include "sound_emitter.h"
#include "sound_listener_info.h"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <limits>
#include <optional>
#include <thread>
#include <aaudio/AAudio.h>


GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class SoundMixer final
{
    private:
        using Emitters = std::unordered_map<AAudioStream*, SoundEmitter*>;
        using Decompressors = std::unordered_map<AAudioStream*, PCMStreamer*>;

    private:
        constexpr static auto           TOTAL_SOUND_CHANNELS = static_cast<size_t> ( eSoundChannel::Speech ) + 1U;

        int32_t                         _bufferFrameCount = std::numeric_limits<int32_t>::min ();
        size_t                          _bufferSampleCount = std::numeric_limits<size_t>::max ();
        AAudioStreamBuilder*            _builder = nullptr;
        float                           _channelVolume[ TOTAL_SOUND_CHANNELS ] {};

        bool                            _decompressorFlag = true;
        std::thread                     _decompressorThread {};
        Decompressors                   _decompressors {};

        float                           _effectiveChannelVolume[ TOTAL_SOUND_CHANNELS ] {};
        Emitters                        _emitters {};
        std::deque<SoundEmitter*>       _emittersToResume {};


        SoundListenerInfo               _listenerInfo {};
        GXQuat                          _listenerOrientation {};
        bool                            _listenerTransformChanged = true;

        float                           _masterVolume = 1.0F;
        std::mutex                      _mutex {};

    public:
        SoundMixer () = default;

        SoundMixer ( SoundMixer const & ) = delete;
        SoundMixer& operator = ( SoundMixer const & ) = delete;

        SoundMixer ( SoundMixer && ) = delete;
        SoundMixer& operator = ( SoundMixer && ) = delete;

        ~SoundMixer () = default;

        [[nodiscard]] bool Init () noexcept;
        void Destroy () noexcept;

        // Note SoundMixer is not owned AAudioStream. The callee MUST make AAudioStream_close call.
        [[nodiscard]] std::optional<AAudioStream*> CreateStream ( SoundEmitter::Context &context ) noexcept;
        [[nodiscard]] bool DestroyStream ( AAudioStream &stream ) noexcept;

        // Note this is exact amount of samples for all channels in total.
        // For example stereo asset with 7 frames will return 14 samples.
        [[maybe_unused, nodiscard]] size_t GetBufferSampleCount () const noexcept;

        [[maybe_unused, nodiscard]] float GetChannelVolume ( eSoundChannel channel ) const noexcept;
        [[maybe_unused]] void SetChannelVolume ( eSoundChannel channel, float volume ) noexcept;

        [[maybe_unused, nodiscard]] float GetMasterVolume () const noexcept;
        void SetMasterVolume ( float volume ) noexcept;

        [[nodiscard]] SoundListenerInfo const& GetListenerInfo () noexcept;
        [[maybe_unused]] void SetListenerLocation ( GXVec3 const &location ) noexcept;
        [[maybe_unused]] void SetListenerOrientation ( GXQuat const &orientation ) noexcept;

        void Pause () noexcept;
        void Resume () noexcept;

        void RegisterDecompressor ( AAudioStream &stream, PCMStreamer &streamer ) noexcept;
        void UnregisterDecompressor ( AAudioStream &stream ) noexcept;

        // Method returns true if "result" equals AAUDIO_OK. Otherwise method returns false.
        static bool CheckAAudioResult ( aaudio_result_t result, char const* from, char const* message ) noexcept;

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
        [[nodiscard]] std::optional<AAudioStream*> CreateStreamInternal ( SoundEmitter::Context &context ) noexcept;
        [[nodiscard]] bool ResolveBufferSize () noexcept;
        void RecreateSoundEmitter ( AAudioStream &stream ) noexcept;

        static void ErrorCallback ( AAudioStream* stream, void* userData, aaudio_result_t err );

        [[nodiscard]] static aaudio_data_callback_result_t PCMCallback ( AAudioStream* stream,
            void* userData,
            void* audioData,
            int32_t numFrames
        );

        static void PrintStreamInfo ( AAudioStream &stream, char const* header ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_MIXER_H
