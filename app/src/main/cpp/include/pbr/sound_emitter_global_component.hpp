#ifndef PBR_SOUND_EMITTER_GLOBAL_COMPONENT_HPP
#define PBR_SOUND_EMITTER_GLOBAL_COMPONENT_HPP


#include "actor.hpp"
#include "component.hpp"
#include <sound_emitter_global.hpp>
#include "sound_emitter_global_component_desc.hpp"


namespace pbr {

class SoundEmitterGlobalComponent final : public Component
{
    private:
        Actor*                                                          _actor = nullptr;
        android_vulkan::SoundEmitterGlobal                              _soundEmitter {};

        static int                                                      _registerComponentIndex;
        static std::unordered_map<Component const*, ComponentRef>       _soundEmitters;
        static android_vulkan::SoundMixer*                              _soundMixer;

    public:
        SoundEmitterGlobalComponent () = delete;

        SoundEmitterGlobalComponent ( SoundEmitterGlobalComponent const & ) = delete;
        SoundEmitterGlobalComponent &operator = ( SoundEmitterGlobalComponent const & ) = delete;

        SoundEmitterGlobalComponent ( SoundEmitterGlobalComponent && ) = delete;
        SoundEmitterGlobalComponent &operator = ( SoundEmitterGlobalComponent && ) = delete;

        explicit SoundEmitterGlobalComponent ( bool &success,
            SoundEmitterGlobalComponentDesc const &desc,
            uint8_t const* data
        ) noexcept;

        explicit SoundEmitterGlobalComponent ( android_vulkan::eSoundChannel channel, std::string &&name ) noexcept;

        ~SoundEmitterGlobalComponent () override = default;

        [[nodiscard]] bool IsPlaying () const noexcept;
        [[nodiscard]] bool Pause () noexcept;
        [[nodiscard]] bool Play () noexcept;
        [[nodiscard]] bool Stop () noexcept;
        [[nodiscard]] bool SetSoundAsset ( std::string_view const file, bool looped ) noexcept;

        [[nodiscard]] float GetVolume () const noexcept;
        void SetVolume ( float volume ) noexcept;

        [[nodiscard]] bool RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept;
        void RegisterFromScript ( Actor &actor ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm, android_vulkan::SoundMixer &soundMixer ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] ComponentRef &GetReference () noexcept override;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnGetVolume ( lua_State* state );
        [[nodiscard]] static int OnIsPlaying ( lua_State* state );
        [[nodiscard]] static int OnPause ( lua_State* state );
        [[nodiscard]] static int OnPlay ( lua_State* state );
        [[nodiscard]] static int OnSetSoundAsset ( lua_State* state );
        [[nodiscard]] static int OnSetVolume ( lua_State* state );
        [[nodiscard]] static int OnStop ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SOUND_EMITTER_GLOBAL_COMPONENT_HPP
