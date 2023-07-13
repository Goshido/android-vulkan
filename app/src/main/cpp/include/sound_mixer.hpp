#ifndef ANDROID_VULKAN_SOUND_MIXER_HPP
#define ANDROID_VULKAN_SOUND_MIXER_HPP


#include "sound_emitter.hpp"
#include "sound_listener_info.hpp"
#include "sound_storage.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <limits>
#include <list>
#include <optional>
#include <thread>
#include <unordered_set>
#include <aaudio/AAudio.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

// Class is taking into consideration AAudio limitations.
// See docs/aaudio-issues.md
class SoundMixer final
{
    private:
        using Decompressors = std::unordered_set<PCMStreamer*>;
        using ActionHandler = void ( * ) ( AAudioStream &stream ) noexcept;

        struct ActionInfo final
        {
            ActionHandler                               _handler = nullptr;
            AAudioStream*                               _stream = nullptr;
        };

        class StreamInfo;
        using StreamList = std::list<StreamInfo*>;

        class StreamInfo final
        {
            private:
                std::unique_ptr<std::atomic_bool>       _fillLock = std::make_unique<std::atomic_bool> ( false );
                bool                                    _fillSilence = true;

            public:
                SoundEmitter*                           _emitter = nullptr;
                SoundMixer*                             _mixer = nullptr;
                AAudioStream*                           _stream = nullptr;
                StreamList::iterator                    _used {};

            public:
                StreamInfo () = default;

                StreamInfo ( StreamInfo const & ) = delete;
                StreamInfo &operator = ( StreamInfo const & ) = delete;

                StreamInfo ( StreamInfo && ) = default;
                StreamInfo &operator = ( StreamInfo && ) = delete;

                explicit StreamInfo ( SoundMixer &mixer, StreamList::iterator used ) noexcept;

                ~StreamInfo () = default;

                // This is part of AAudio fill thread sync.
                // AAudio fill thread should be mutex free to avoid audio glitches.
                [[nodiscard]] bool LockAndReturnFillSilence () noexcept;
                void Release () noexcept;
                void Modify ( bool fillSilence ) noexcept;
        };

        using StreamMap = std::unordered_map<SoundEmitter*, StreamInfo*>;

    private:
        constexpr static auto TOTAL_SOUND_CHANNELS = static_cast<size_t> ( eSoundChannel::Speech ) + 1U;

        std::vector<ActionInfo>                         _actionQueue {};

        int32_t                                         _bufferFrameCount = std::numeric_limits<int32_t>::min ();
        size_t                                          _bufferSampleCount = std::numeric_limits<size_t>::max ();
        AAudioStreamBuilder*                            _builder = nullptr;
        float                                           _channelVolume[ TOTAL_SOUND_CHANNELS ] {};
        Decompressors                                   _decompressors {};

        float                                           _effectiveChannelVolume[ TOTAL_SOUND_CHANNELS ] {};

        SoundListenerInfo                               _listenerInfo {};
        GXQuat                                          _listenerOrientation {};
        bool                                            _listenerTransformChanged = true;

        float                                           _masterVolume = 1.0F;
        std::mutex                                      _mutex {};

        SoundStorage                                    _soundStorage {};

        StreamMap                                       _streamMap {};
        std::vector<StreamInfo>                         _streamInfo {};
        std::deque<AAudioStream*>                       _streamToResume {};
        StreamList                                      _free {};
        StreamList                                      _used {};

        bool                                            _workerFlag = true;
        std::thread                                     _workerThread {};

    public:
        SoundMixer () = default;

        SoundMixer ( SoundMixer const & ) = delete;
        SoundMixer &operator = ( SoundMixer const & ) = delete;

        SoundMixer ( SoundMixer && ) = delete;
        SoundMixer &operator = ( SoundMixer && ) = delete;

        ~SoundMixer () = default;

        void CheckMemoryLeaks () noexcept;

        [[nodiscard]] bool Init () noexcept;
        void Destroy () noexcept;

        // Note this is exact amount of samples for all channels in total.
        // For example stereo asset with 7 frames will return 14 samples.
        [[nodiscard]] size_t GetBufferSampleCount () const noexcept;

        [[maybe_unused, nodiscard]] float GetChannelVolume ( eSoundChannel channel ) const noexcept;
        void SetChannelVolume ( eSoundChannel channel, float volume ) noexcept;

        [[maybe_unused, nodiscard]] float GetMasterVolume () const noexcept;
        void SetMasterVolume ( float volume ) noexcept;

        [[nodiscard]] SoundListenerInfo const &GetListenerInfo () noexcept;
        void SetListenerLocation ( GXVec3 const &location ) noexcept;
        void SetListenerOrientation ( GXQuat const &orientation ) noexcept;

        [[nodiscard]] SoundStorage &GetSoundStorage () noexcept;

        [[nodiscard]] bool IsOffline () const noexcept;

        void Pause () noexcept;
        void Resume () noexcept;

        [[nodiscard]] bool RequestPause ( SoundEmitter &emitter ) noexcept;
        [[nodiscard]] bool RequestPlay ( SoundEmitter &emitter ) noexcept;
        [[nodiscard]] bool RequestStop ( SoundEmitter &emitter ) noexcept;

        void RegisterDecompressor ( PCMStreamer &streamer ) noexcept;
        void UnregisterDecompressor ( PCMStreamer &streamer ) noexcept;

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
        [[nodiscard]] bool CreateHardwareStreams () noexcept;
        [[nodiscard]] std::optional<AAudioStream*> CreateStream ( StreamInfo &streamInfo ) noexcept;

        void RecreateSoundStream ( AAudioStream &stream ) noexcept;
        [[nodiscard]] bool ResolveBufferSize () noexcept;
        [[nodiscard]] bool SetStreamBufferSize ( AAudioStream &stream, char const* where ) const noexcept;
        [[nodiscard]] bool ValidateStream ( AAudioStream &stream ) const noexcept;

        static void ErrorCallback ( AAudioStream* stream, void* userData, aaudio_result_t err );

        static void ExecutePause ( AAudioStream &stream ) noexcept;
        static void ExecutePlay ( AAudioStream &stream ) noexcept;
        static void ExecuteStop ( AAudioStream &stream ) noexcept;

        [[nodiscard]] static aaudio_data_callback_result_t PCMCallback ( AAudioStream* stream,
            void* userData,
            void* audioData,
            int32_t numFrames
        );

        static void PrintStreamInfo ( AAudioStream &stream, char const* header ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_MIXER_HPP
