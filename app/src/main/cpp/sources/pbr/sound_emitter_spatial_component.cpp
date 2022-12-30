#include <pbr/sound_emitter_spatial_component.h>
#include <pbr/script_engine.h>
#include <pbr/scriptable_gxvec3.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

[[maybe_unused]] constexpr uint32_t SOUND_EMITTER_SPATIAL_COMPONENT_DESC_FORMAT_VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

int SoundEmitterSpatialComponent::_registerComponentIndex = std::numeric_limits<int>::max ();
std::unordered_map<Component const*, ComponentRef> SoundEmitterSpatialComponent::_soundEmitters {};
android_vulkan::SoundMixer* SoundEmitterSpatialComponent::_soundMixer = nullptr;

SoundEmitterSpatialComponent::SoundEmitterSpatialComponent ( bool &success,
    SoundEmitterSpatialComponentDesc const &desc,
    uint8_t const* data
) noexcept:
    Component ( ClassID::SoundEmitterSpatial )
{
    // Sanity checks.
    static_assert ( sizeof ( desc._location ) == sizeof ( GXVec3 ) );
    assert ( desc._formatVersion == SOUND_EMITTER_SPATIAL_COMPONENT_DESC_FORMAT_VERSION );

    _name = reinterpret_cast<char const*> ( data + desc._name );

    if ( !_soundEmitter.Init ( *_soundMixer, static_cast<android_vulkan::eSoundChannel> ( desc._channel ) ) )
    {
        success = false;
        return;
    }

    _soundEmitter.SetDistance ( desc._distance );
    _soundEmitter.SetLocation ( *reinterpret_cast<GXVec3 const*> ( &desc._location ) );
    _soundEmitter.SetVolume ( desc._volume );

    if ( desc._soundAsset == android_vulkan::NO_UTF8_OFFSET )
    {
        success = true;
        return;
    }

    auto const* asset = reinterpret_cast<char const *> ( desc._soundAsset );

    if ( _soundEmitter.SetSoundAsset ( asset, static_cast<bool> ( desc._looped ) ) )
    {
        success = true;
        return;
    }

    if ( !_soundEmitter.Destroy () )
    {
        android_vulkan::LogWarning ( "SoundEmitterSpatialComponent::SoundEmitterSpatialComponent - "
            "Can't destroy sound emitter. Asset: %s",
            asset
        );
    }

    success = false;
}

SoundEmitterSpatialComponent::SoundEmitterSpatialComponent ( bool &success,
    android_vulkan::eSoundChannel channel,
    std::string &&name
) noexcept:
    Component ( ClassID::SoundEmitterSpatial, std::move ( name ) )
{
    success = _soundEmitter.Init ( *_soundMixer, channel );
}

bool SoundEmitterSpatialComponent::IsPlaying () const noexcept
{
    return _soundEmitter.IsPlaying ();
}

bool SoundEmitterSpatialComponent::Pause () noexcept
{
    return _soundEmitter.Pause ();
}

bool SoundEmitterSpatialComponent::Play () noexcept
{
    return _soundEmitter.Play ();
}

bool SoundEmitterSpatialComponent::Stop () noexcept
{
    return _soundEmitter.Stop ();
}

void SoundEmitterSpatialComponent::SetDistance ( float distance ) noexcept
{
    _soundEmitter.SetDistance ( distance );
}

void SoundEmitterSpatialComponent::SetLocation ( GXVec3 const &location ) noexcept
{
    _soundEmitter.SetLocation ( location );
}

bool SoundEmitterSpatialComponent::SetSoundAsset ( std::string_view const file, bool looped ) noexcept
{
    return _soundEmitter.SetSoundAsset ( file, looped );
}

float SoundEmitterSpatialComponent::GetVolume () const noexcept
{
    return _soundEmitter.GetVolume ();
}

void SoundEmitterSpatialComponent::SetVolume ( float volume ) noexcept
{
    _soundEmitter.SetVolume ( volume );
}

bool SoundEmitterSpatialComponent::RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept
{
    _actor = &actor;

    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::SoundEmitterSpatialComponent::RegisterFromNative - Stack too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

void SoundEmitterSpatialComponent::RegisterFromScript ( Actor &actor ) noexcept
{
    _actor = &actor;
}

bool SoundEmitterSpatialComponent::Init ( lua_State &vm, android_vulkan::SoundMixer &soundMixer ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) )
    {
        android_vulkan::LogError ( "pbr::SoundEmitterSpatialComponent::Init - Stack too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterSoundEmitterSpatialComponent" ) != LUA_TFUNCTION )
    {
        android_vulkan::LogError ( "pbr::SoundEmitterSpatialComponent::Init - Can't find register function." );
        return false;
    }

    _registerComponentIndex = lua_gettop ( &vm );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_SoundEmitterSpatialComponentCreate",
            .func = &SoundEmitterSpatialComponent::OnCreate
        },
        {
            .name = "av_SoundEmitterSpatialComponentDestroy",
            .func = &SoundEmitterSpatialComponent::OnDestroy
        },
        {
            .name = "av_SoundEmitterSpatialComponentCollectGarbage",
            .func = &SoundEmitterSpatialComponent::OnGarbageCollected
        },
        {
            .name = "av_SoundEmitterSpatialComponentGetVolume",
            .func = &SoundEmitterSpatialComponent::OnGetVolume
        },
        {
            .name = "av_SoundEmitterSpatialComponentIsPlaying",
            .func = &SoundEmitterSpatialComponent::OnIsPlaying
        },
        {
            .name = "av_SoundEmitterSpatialComponentPause",
            .func = &SoundEmitterSpatialComponent::OnPause
        },
        {
            .name = "av_SoundEmitterSpatialComponentPlay",
            .func = &SoundEmitterSpatialComponent::OnPlay
        },
        {
            .name = "av_SoundEmitterSpatialComponentSetDistance",
            .func = &SoundEmitterSpatialComponent::OnSetDistance
        },
        {
            .name = "av_SoundEmitterSpatialComponentSetLocation",
            .func = &SoundEmitterSpatialComponent::OnSetLocation
        },
        {
            .name = "av_SoundEmitterSpatialComponentSetSoundAsset",
            .func = &SoundEmitterSpatialComponent::OnSetSoundAsset
        },
        {
            .name = "av_SoundEmitterSpatialComponentSetVolume",
            .func = &SoundEmitterSpatialComponent::OnSetVolume
        },
        {
            .name = "av_SoundEmitterSpatialComponentStop",
            .func = &SoundEmitterSpatialComponent::OnStop
        }
    };

    for ( auto const& extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    _soundMixer = &soundMixer;
    return true;
}

void SoundEmitterSpatialComponent::Destroy () noexcept
{
    _soundEmitters.clear ();
}

ComponentRef& SoundEmitterSpatialComponent::GetReference () noexcept
{
    auto findResult = _soundEmitters.find ( this );
    assert ( findResult != _soundEmitters.end () );
    return findResult->second;
}

int SoundEmitterSpatialComponent::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterSpatialComponent::OnCreate - Stack too small." );
        return 0;
    }

    char const* name = lua_tostring ( state, 1 );

    if ( !name )
    {
        lua_pushnil ( state );
        return 1;
    }

    bool success;

    ComponentRef staticMesh = std::make_shared<SoundEmitterSpatialComponent> ( success,
        static_cast<android_vulkan::eSoundChannel> ( lua_tointeger ( state, 3 ) ),
        name
    );

    if ( !success )
    {
        lua_pushnil ( state );
        return 1;
    }

    Component* handle = staticMesh.get ();
    _soundEmitters.emplace ( handle, std::move ( staticMesh ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int SoundEmitterSpatialComponent::OnDestroy ( lua_State* state )
{
    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    self._actor->DestroyComponent ( self );
    return 0;
}

int SoundEmitterSpatialComponent::OnGarbageCollected ( lua_State* state )
{
    // NOLINTNEXTLINE - downcast.
    auto* component = static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );

    if ( !component->_soundEmitter.Destroy () )
        android_vulkan::LogWarning ( "SoundEmitterSpatialComponent::OnGarbageCollected - Can't destroy emitter." );

    _soundEmitters.erase ( component );
    return 0;
}

int SoundEmitterSpatialComponent::OnGetVolume ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterSpatialComponent::OnGetVolume - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<SoundEmitterSpatialComponent const*> ( lua_touserdata ( state, 1 ) );
    lua_pushnumber ( state, static_cast<lua_Number> ( self.GetVolume () ) );
    return 1;
}

int SoundEmitterSpatialComponent::OnIsPlaying ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterSpatialComponent::OnIsPlaying - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<SoundEmitterSpatialComponent const*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.IsPlaying () );
    return 1;
}

int SoundEmitterSpatialComponent::OnPause ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterSpatialComponent::OnPause - Stack too small." );
        return 0;
    }

    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.Pause () );
    return 1;
}

int SoundEmitterSpatialComponent::OnPlay ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterSpatialComponent::OnPlay - Stack too small." );
        return 0;
    }

    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.Play () );
    return 1;
}

int SoundEmitterSpatialComponent::OnSetDistance ( lua_State* state )
{
    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetDistance ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int SoundEmitterSpatialComponent::OnSetLocation ( lua_State* state )
{
    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetLocation ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int SoundEmitterSpatialComponent::OnSetSoundAsset ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::OnSetSoundAsset::OnStop - Stack too small." );
        return 0;
    }

    size_t len;
    char const* soundAsset = lua_tolstring ( state, 2, &len );

    if ( !soundAsset )
    {
        lua_pushboolean ( state, false );
        return 1;
    }

    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.SetSoundAsset ( std::string_view ( soundAsset, len ), lua_toboolean ( state, 3 ) ) );
    return 1;
}

int SoundEmitterSpatialComponent::OnSetVolume ( lua_State* state )
{
    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetVolume ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int SoundEmitterSpatialComponent::OnStop ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SoundEmitterSpatialComponent::OnStop - Stack too small." );
        return 0;
    }

    auto& self = *static_cast<SoundEmitterSpatialComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.Stop () );
    return 1;
}

} // namespace pbr
