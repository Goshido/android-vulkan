#include <pbr/sound_emitter_global_component.hpp>
#include <pbr/script_engine.hpp>
#include <av_assert.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

[[maybe_unused]] constexpr uint32_t SOUND_EMITTER_GLOBAL_COMPONENT_DESC_FORMAT_VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

int SoundEmitterGlobalComponent::_registerComponentIndex = std::numeric_limits<int>::max ();
std::unordered_map<Component const*, ComponentRef> SoundEmitterGlobalComponent::_soundEmitters {};
android_vulkan::SoundMixer* SoundEmitterGlobalComponent::_soundMixer = nullptr;

SoundEmitterGlobalComponent::SoundEmitterGlobalComponent ( bool &success,
    SoundEmitterGlobalComponentDesc const &desc,
    uint8_t const* data
) noexcept:
    Component ( ClassID::SoundEmitterGlobal )
{
    // Sanity check.
    AV_ASSERT ( desc._formatVersion == SOUND_EMITTER_GLOBAL_COMPONENT_DESC_FORMAT_VERSION )
    _name = reinterpret_cast<char const*> ( data + desc._name );

    _soundEmitter.Init ( *_soundMixer, static_cast<android_vulkan::eSoundChannel> ( desc._channel ) );
    _soundEmitter.SetVolume ( desc._volume );

    if ( desc._soundAsset == android_vulkan::NO_UTF8_OFFSET )
    {
        success = true;
        return;
    }

    auto const* asset = reinterpret_cast<char const *> ( desc._soundAsset );

    if ( _soundEmitter.SetSoundAsset ( asset, static_cast<bool> ( desc._looped ) ) ) [[likely]]
    {
        success = true;
        return;
    }

    if ( !_soundEmitter.Destroy () )
    {
        android_vulkan::LogWarning ( "SoundEmitterGlobalComponent::SoundEmitterGlobalComponent - "
            "Can't destroy sound emitter. Asset: %s",
            asset
        );
    }

    success = false;
}

SoundEmitterGlobalComponent::SoundEmitterGlobalComponent ( android_vulkan::eSoundChannel channel,
    std::string &&name
) noexcept:
    Component ( ClassID::SoundEmitterGlobal, std::move ( name ) )
{
    _soundEmitter.Init ( *_soundMixer, channel );
}

bool SoundEmitterGlobalComponent::IsPlaying () const noexcept
{
    return _soundEmitter.IsPlaying ();
}

bool SoundEmitterGlobalComponent::Pause () noexcept
{
    return _soundEmitter.Pause ();
}

bool SoundEmitterGlobalComponent::Play () noexcept
{
    return _soundEmitter.Play ();
}

bool SoundEmitterGlobalComponent::Stop () noexcept
{
    return _soundEmitter.Stop ();
}

bool SoundEmitterGlobalComponent::SetSoundAsset ( std::string_view const file, bool looped ) noexcept
{
    return _soundEmitter.SetSoundAsset ( file, looped );
}

float SoundEmitterGlobalComponent::GetVolume () const noexcept
{
    return _soundEmitter.GetVolume ();
}

void SoundEmitterGlobalComponent::SetVolume ( float volume ) noexcept
{
    _soundEmitter.SetVolume ( volume );
}

bool SoundEmitterGlobalComponent::RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept
{
    _actor = &actor;

    if ( !lua_checkstack ( &vm, 2 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::SoundEmitterGlobalComponent::RegisterFromNative - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

void SoundEmitterGlobalComponent::RegisterFromScript ( Actor &actor ) noexcept
{
    _actor = &actor;
}

bool SoundEmitterGlobalComponent::Init ( lua_State &vm, android_vulkan::SoundMixer &soundMixer ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::SoundEmitterGlobalComponent::Init - Stack is too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterSoundEmitterGlobalComponent" ) != LUA_TFUNCTION ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::SoundEmitterGlobalComponent::Init - Can't find register function." );
        return false;
    }

    _registerComponentIndex = lua_gettop ( &vm );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_SoundEmitterGlobalComponentCreate",
            .func = &SoundEmitterGlobalComponent::OnCreate
        },
        {
            .name = "av_SoundEmitterGlobalComponentDestroy",
            .func = &SoundEmitterGlobalComponent::OnDestroy
        },
        {
            .name = "av_SoundEmitterGlobalComponentCollectGarbage",
            .func = &SoundEmitterGlobalComponent::OnGarbageCollected
        },
        {
            .name = "av_SoundEmitterGlobalComponentGetVolume",
            .func = &SoundEmitterGlobalComponent::OnGetVolume
        },
        {
            .name = "av_SoundEmitterGlobalComponentIsPlaying",
            .func = &SoundEmitterGlobalComponent::OnIsPlaying
        },
        {
            .name = "av_SoundEmitterGlobalComponentPause",
            .func = &SoundEmitterGlobalComponent::OnPause
        },
        {
            .name = "av_SoundEmitterGlobalComponentPlay",
            .func = &SoundEmitterGlobalComponent::OnPlay
        },
        {
            .name = "av_SoundEmitterGlobalComponentSetSoundAsset",
            .func = &SoundEmitterGlobalComponent::OnSetSoundAsset
        },
        {
            .name = "av_SoundEmitterGlobalComponentSetVolume",
            .func = &SoundEmitterGlobalComponent::OnSetVolume
        },
        {
            .name = "av_SoundEmitterGlobalComponentStop",
            .func = &SoundEmitterGlobalComponent::OnStop
        }
    };

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    _soundMixer = &soundMixer;
    return true;
}

void SoundEmitterGlobalComponent::Destroy () noexcept
{
    if ( !_soundEmitters.empty () ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterGlobalComponent::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _soundEmitters.clear ();
}

ComponentRef &SoundEmitterGlobalComponent::GetReference () noexcept
{
    auto findResult = _soundEmitters.find ( this );
    AV_ASSERT ( findResult != _soundEmitters.end () )
    return findResult->second;
}

int SoundEmitterGlobalComponent::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterGlobalComponent::OnCreate - Stack is too small." );
        return 0;
    }

    char const* name = lua_tostring ( state, 1 );

    if ( !name ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    ComponentRef staticMesh = std::make_shared<SoundEmitterGlobalComponent> (
        static_cast<android_vulkan::eSoundChannel> ( lua_tointeger ( state, 3 ) ),
        name
    );

    Component* handle = staticMesh.get ();
    _soundEmitters.emplace ( handle, std::move ( staticMesh ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int SoundEmitterGlobalComponent::OnDestroy ( lua_State* state )
{
    auto &self = *static_cast<SoundEmitterGlobalComponent*> ( lua_touserdata ( state, 1 ) );
    self._actor->DestroyComponent ( self );
    return 0;
}

int SoundEmitterGlobalComponent::OnGarbageCollected ( lua_State* state )
{
    // NOLINTNEXTLINE - downcast.
    auto* component = static_cast<SoundEmitterGlobalComponent*> ( lua_touserdata ( state, 1 ) );

    if ( !component->_soundEmitter.Destroy () ) [[unlikely]]
        android_vulkan::LogWarning ( "SoundEmitterGlobalComponent::OnGarbageCollected - Can't destroy emitter." );

    _soundEmitters.erase ( component );
    return 0;
}

int SoundEmitterGlobalComponent::OnGetVolume ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterGlobalComponent::OnGetVolume - Stack is too small." );
        return 0;
    }

    auto const &self = *static_cast<SoundEmitterGlobalComponent const*> ( lua_touserdata ( state, 1 ) );
    lua_pushnumber ( state, static_cast<lua_Number> ( self.GetVolume () ) );
    return 1;
}

int SoundEmitterGlobalComponent::OnIsPlaying ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterGlobalComponent::OnIsPlaying - Stack is too small." );
        return 0;
    }

    auto const &self = *static_cast<SoundEmitterGlobalComponent const*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.IsPlaying () );
    return 1;
}

int SoundEmitterGlobalComponent::OnPause ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterGlobalComponent::OnPause - Stack is too small." );
        return 0;
    }

    auto &self = *static_cast<SoundEmitterGlobalComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.Pause () );
    return 1;
}

int SoundEmitterGlobalComponent::OnPlay ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterGlobalComponent::OnPlay - Stack is too small." );
        return 0;
    }

    auto &self = *static_cast<SoundEmitterGlobalComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.Play () );
    return 1;
}

int SoundEmitterGlobalComponent::OnSetSoundAsset ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::OnSetSoundAsset::OnStop - Stack is too small." );
        return 0;
    }

    size_t len;
    char const* soundAsset = lua_tolstring ( state, 2, &len );

    if ( !soundAsset ) [[unlikely]]
    {
        lua_pushboolean ( state, false );
        return 1;
    }

    auto &self = *static_cast<SoundEmitterGlobalComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.SetSoundAsset ( std::string_view ( soundAsset, len ), lua_toboolean ( state, 3 ) ) );
    return 1;
}

int SoundEmitterGlobalComponent::OnSetVolume ( lua_State* state )
{
    auto &self = *static_cast<SoundEmitterGlobalComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetVolume ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int SoundEmitterGlobalComponent::OnStop ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterGlobalComponent::OnStop - Stack is too small." );
        return 0;
    }

    auto &self = *static_cast<SoundEmitterGlobalComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.Stop () );
    return 1;
}

} // namespace pbr
