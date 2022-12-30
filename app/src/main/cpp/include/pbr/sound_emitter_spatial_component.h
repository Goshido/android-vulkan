#ifndef PBR_SOUND_EMITTER_SPATIAL_COMPONENT_H
#define PBR_SOUND_EMITTER_SPATIAL_COMPONENT_H


#include "actor.h"
#include "component.h"
#include <sound_emitter_spatial.h>
#include "sound_emitter_spatial_component_desc.h"


namespace pbr {

class SoundEmitterSpatialComponent final : public Component
{
    private:
        Actor*                                                          _actor = nullptr;
        android_vulkan::SoundEmitterSpatial                             _soundEmitter {};

        static int                                                      _registerComponentIndex;
        static std::unordered_map<Component const*, ComponentRef>       _soundEmitters;
        static android_vulkan::SoundMixer*                              _soundMixer;

    public:
        SoundEmitterSpatialComponent () = delete;

        SoundEmitterSpatialComponent ( SoundEmitterSpatialComponent const & ) = delete;
        SoundEmitterSpatialComponent& operator = ( SoundEmitterSpatialComponent const & ) = delete;

        SoundEmitterSpatialComponent ( SoundEmitterSpatialComponent && ) = delete;
        SoundEmitterSpatialComponent& operator = ( SoundEmitterSpatialComponent && ) = delete;

        explicit SoundEmitterSpatialComponent ( bool &success,
            SoundEmitterSpatialComponentDesc const &desc,
            uint8_t const* data
        ) noexcept;

        explicit SoundEmitterSpatialComponent ( bool &success,
            android_vulkan::eSoundChannel channel,
            std::string &&name
        ) noexcept;

        ~SoundEmitterSpatialComponent () override = default;

        [[nodiscard]] bool IsPlaying () const noexcept;
        [[nodiscard]] bool Pause () noexcept;
        [[nodiscard]] bool Play () noexcept;
        [[nodiscard]] bool Stop () noexcept;
        void SetDistance ( float distance ) noexcept;
        void SetLocation ( GXVec3 const &location ) noexcept;
        [[nodiscard]] bool SetSoundAsset ( std::string_view const file, bool looped ) noexcept;

        [[nodiscard]] float GetVolume () const noexcept;
        void SetVolume ( float volume ) noexcept;

        [[nodiscard]] bool RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept;
        void RegisterFromScript ( Actor &actor ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm, android_vulkan::SoundMixer &soundMixer ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] ComponentRef& GetReference () noexcept override;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnGetVolume ( lua_State* state );
        [[nodiscard]] static int OnIsPlaying ( lua_State* state );
        [[nodiscard]] static int OnPause ( lua_State* state );
        [[nodiscard]] static int OnPlay ( lua_State* state );
        [[nodiscard]] static int OnSetDistance ( lua_State* state );
        [[nodiscard]] static int OnSetLocation ( lua_State* state );
        [[nodiscard]] static int OnSetSoundAsset ( lua_State* state );
        [[nodiscard]] static int OnSetVolume ( lua_State* state );
        [[nodiscard]] static int OnStop ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SOUND_EMITTER_SPATIAL_COMPONENT_H
