#include <pbr/actor.hpp>
#include <pbr/camera_component.hpp>
#include <pbr/point_light_component.hpp>
#include <pbr/reflection_component.hpp>
#include <pbr/rigid_body_component.hpp>
#include <pbr/scene.hpp>
#include <pbr/script_component.hpp>
#include <pbr/script_engine.hpp>
#include <pbr/sound_emitter_global_component.hpp>
#include <pbr/sound_emitter_spatial_component.hpp>
#include <pbr/static_mesh_component.hpp>
#include <pbr/transform_component.hpp>
#include <guid_generator.hpp>
#include <av_assert.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class Actor::StaticInitializer final
{
    public:
        StaticInitializer () noexcept;

        StaticInitializer ( StaticInitializer const & ) = delete;
        StaticInitializer &operator = ( StaticInitializer const & ) = delete;

        StaticInitializer ( StaticInitializer && ) = delete;
        StaticInitializer &operator = ( StaticInitializer && ) = delete;

        ~StaticInitializer () = default;
};

Actor::StaticInitializer::StaticInitializer () noexcept
{
    auto* destroy = Actor::_destroyHandlers;
    destroy[ static_cast<size_t> ( ClassID::Camera ) ] = &Actor::DestroyComponentStub;
    destroy[ static_cast<size_t> ( ClassID::PointLight ) ] = &Actor::DestroyComponentStub;
    destroy[ static_cast<size_t> ( ClassID::Reflection ) ] = &Actor::DestroyComponentStub;
    destroy[ static_cast<size_t> ( ClassID::RigidBody ) ] = &Actor::DestroyRigidBodyComponent;
    destroy[ static_cast<size_t> ( ClassID::Script ) ] = &Actor::DestroyComponentStub;
    destroy[ static_cast<size_t> ( ClassID::SoundEmitterGlobal ) ] = &Actor::DestroyComponentStub;
    destroy[ static_cast<size_t> ( ClassID::SoundEmitterSpatial ) ] = &Actor::DestroyComponentStub;
    destroy[ static_cast<size_t> ( ClassID::StaticMesh ) ] = &Actor::DestroyStaticMeshComponent;
    destroy[ static_cast<size_t> ( ClassID::Transform ) ] = &Actor::DestroyComponentStub;
    destroy[ static_cast<size_t> ( ClassID::Unknown ) ] = &Actor::DestroyComponentStub;

    auto* native = Actor::_registerFromNativeHandlers;
    native[ static_cast<size_t> ( ClassID::Camera ) ] = &Actor::AppendCameraComponentFromNative;
    native[ static_cast<size_t> ( ClassID::PointLight ) ] = &Actor::AppendPointLightComponentFromNative;
    native[ static_cast<size_t> ( ClassID::Reflection ) ] = &Actor::AppendReflectionComponentFromNative;
    native[ static_cast<size_t> ( ClassID::RigidBody ) ] = &Actor::AppendRigidBodyComponentFromNative;
    native[ static_cast<size_t> ( ClassID::Script ) ] = &Actor::AppendScriptComponentFromNative;
    native[ static_cast<size_t> ( ClassID::SoundEmitterGlobal ) ] = &Actor::AppendSoundEmitterGlobalComponentFromNative;

    native[ static_cast<size_t> ( ClassID::SoundEmitterSpatial ) ] =
        &Actor::AppendSoundEmitterSpatialComponentFromNative;

    native[ static_cast<size_t> ( ClassID::StaticMesh ) ] = &Actor::AppendStaticMeshComponentFromNative;
    native[ static_cast<size_t> ( ClassID::Transform ) ] = &Actor::AppendTransformComponentFromNative;
    native[ static_cast<size_t> ( ClassID::Unknown ) ] = &Actor::AppendUnknownComponentFromNative;

    auto* script = Actor::_registerFromScriptHandlers;
    script[ static_cast<size_t> ( ClassID::Camera ) ] = &Actor::AppendCameraComponentFromScript;
    script[ static_cast<size_t> ( ClassID::PointLight ) ] = &Actor::AppendPointLightComponentFromScript;
    script[ static_cast<size_t> ( ClassID::Reflection ) ] = &Actor::AppendReflectionComponentFromScript;
    script[ static_cast<size_t> ( ClassID::RigidBody ) ] = &Actor::AppendRigidBodyComponentFromScript;
    script[ static_cast<size_t> ( ClassID::Script ) ] = &Actor::AppendScriptComponentFromScript;
    script[ static_cast<size_t> ( ClassID::SoundEmitterGlobal ) ] = &Actor::AppendSoundEmitterGlobalComponentFromScript;

    script[ static_cast<size_t> ( ClassID::SoundEmitterSpatial ) ] =
        &Actor::AppendSoundEmitterSpatialComponentFromScript;

    script[ static_cast<size_t> ( ClassID::StaticMesh ) ] = &Actor::AppendStaticMeshComponentFromScript;
    script[ static_cast<size_t> ( ClassID::Transform ) ] = &Actor::AppendTransformComponentFromScript;
    script[ static_cast<size_t> ( ClassID::Unknown ) ] = &Actor::AppendUnknownComponentFromScript;
}

//----------------------------------------------------------------------------------------------------------------------

namespace {

[[maybe_unused]] constexpr uint32_t ACTOR_DESC_FORMAT_VERSION = 1U;
[[maybe_unused]] Actor::StaticInitializer const g_StaticInitializer {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

int Actor::_appendComponentFromNativeIndex = std::numeric_limits<int>::max ();
int Actor::_makeActorIndex = std::numeric_limits<int>::max ();
Actor::DestroyHander Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::COUNT ) ] = {};
Actor::RegisterFromNativeHander Actor::_registerFromNativeHandlers[ static_cast<size_t> ( ClassID::COUNT ) ] = {};
Actor::RegisterFromScriptHander Actor::_registerFromScriptHandlers[ static_cast<size_t> ( ClassID::COUNT ) ] = {};
Actor::SpawnActors Actor::_spawnActors {};

Actor::Actor ( std::string &&name ) noexcept:
    _name ( std::move ( name ) )
{
    // NOTHING
}

Actor::Actor ( ActorDesc const &desc, uint8_t const* data ) noexcept
{
    // Sanity checks.
    static_assert ( sizeof ( ActorDesc::_localMatrix ) == sizeof ( desc._localMatrix ) );
    AV_ASSERT ( desc._formatVersion == ACTOR_DESC_FORMAT_VERSION )

    _name = reinterpret_cast<char const*> ( data + desc._name );
}

void Actor::AppendComponent ( ComponentRef &component ) noexcept
{
    _components.push_back ( component );
}

void Actor::DestroyComponent ( Component &component ) noexcept
{
    DestroyHander const handler = _destroyHandlers[ static_cast<size_t> ( component.GetClassID () ) ];

    // C++ calling method by pointer syntax.
    ( this->*handler ) ( component );
}

std::string const &Actor::GetName () const noexcept
{
    return _name;
}

void Actor::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    for ( auto &component : _transformableComponents )
    {
        component.get ().OnTransform ( transformWorld );
    }
}

void Actor::RegisterComponentsFromNative ( Scene &scene,
    ComponentList &renderable,
    android_vulkan::Physics &physics,
    lua_State &vm
) noexcept
{
    if ( !lua_checkstack ( &vm, 4 ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::RegisterComponentsFromNative - Stack is too small." );
        return;
    }

    lua_pushvalue ( &vm, _makeActorIndex );
    lua_pushlightuserdata ( &vm, this );

    if ( lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK )
    {
        android_vulkan::LogWarning ( "pbr::Actor::RegisterComponentsFromNative - Can't register actor inside Lua VM." );
        return;
    }

    _scene = &scene;

    for ( auto &component : _components )
    {
        auto const handler = _registerFromNativeHandlers[ static_cast<size_t> ( component->GetClassID () ) ];

        // C++ calling method by pointer syntax.
        ( this->*handler ) ( component, renderable, physics, vm );
    }
}

void Actor::RegisterComponentsFromScript ( Scene &scene,
    ComponentList &renderable,
    android_vulkan::Physics &physics
) noexcept
{
    _scene = &scene;

    for ( auto &component : _components )
    {
        auto const handler = _registerFromScriptHandlers[ static_cast<size_t> ( component->GetClassID () ) ];

        // C++ calling method by pointer syntax.
        ( this->*handler ) ( component, renderable, physics );
    }
}

bool Actor::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_ActorAppendComponent",
            .func = &Actor::OnAppendComponent
        },
        {
            .name = "av_ActorCreate",
            .func = &Actor::OnCreate
        },
        {
            .name = "av_ActorDestroy",
            .func = &Actor::OnDestroy
        },
        {
            .name = "av_ActorCollectGarbage",
            .func = &Actor::OnGarbageCollected
        },
        {
            .name = "av_ActorGetName",
            .func = &Actor::OnGetName
        }
    };

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::Actor::Init - Stack is too small." );
        return false;
    }

    auto bind = [ & ] ( char const* function, int &ind ) noexcept -> bool {
        if ( lua_getglobal ( &vm, function ) == LUA_TFUNCTION )
        {
            ind = lua_gettop ( &vm );
            return true;
        }

        android_vulkan::LogError ( "pbr::Actor::Init - Can't find %s function.", function );
        return false;
    };

    if ( !bind ( "AppendComponentFromNative", _appendComponentFromNativeIndex ) )
        return false;

    return bind ( "MakeActor", _makeActorIndex );
}

void Actor::Destroy () noexcept
{
    if ( !_spawnActors.empty () )
    {
        android_vulkan::LogWarning ( "pbr::Actor::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _spawnActors.clear ();
}

ActorRef &Actor::GetReference ( Actor const &handle ) noexcept
{
    auto findResult = _spawnActors.find ( &handle );
    AV_ASSERT ( findResult != _spawnActors.end () )
    return findResult->second;
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendCameraComponentFromNative ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &cameraComponent = static_cast<CameraComponent &> ( *component );

    lua_pushvalue ( &vm, _appendComponentFromNativeIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !cameraComponent.Register ( vm ) )
    {
        android_vulkan::LogWarning (
            "pbr::Actor::AppendCameraComponentFromNative - Can't register camera component %s.",
            cameraComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendCameraComponentFromNative - Can't append camera component %s inside Lua VM.",
        cameraComponent.GetName ().c_str ()
    );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendCameraComponentFromScript ( ComponentRef &/*component*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // TODO
    AV_ASSERT ( false )
}

void Actor::AppendPointLightComponentFromNative ( ComponentRef &component,
    ComponentList &renderable,
    android_vulkan::Physics &/*physics*/,
    lua_State &/*vm*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &transformable = static_cast<PointLightComponent &> ( *component );

    renderable.emplace_back ( std::ref ( transformable ) );
    _transformableComponents.emplace_back ( std::ref ( transformable ) );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendPointLightComponentFromScript ( ComponentRef &/*component*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // TODO
    AV_ASSERT ( false )
}

void Actor::AppendReflectionComponentFromNative ( ComponentRef &component,
    ComponentList &renderable,
    android_vulkan::Physics &/*physics*/,
    lua_State &/*vm*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &reflection = static_cast<ReflectionComponent  &> ( *component );

    renderable.emplace_back ( reflection );

    if ( !reflection.IsGlobalReflection () )
    {
        _transformableComponents.emplace_back ( std::ref ( reflection ) );
    }
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendReflectionComponentFromScript ( ComponentRef &/*component*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // TODO
    AV_ASSERT ( false )
}

void Actor::AppendRigidBodyComponentFromNative ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &physics,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &rigidBodyComponent = static_cast<RigidBodyComponent &> ( *component );

    lua_pushvalue ( &vm, _appendComponentFromNativeIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !rigidBodyComponent.RegisterFromNative ( *this, physics, vm ) )
    {
        android_vulkan::LogWarning (
            "pbr::Actor::AppendRigidBodyComponentFromNative - Can't register rigid body component %s.",
            rigidBodyComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendRigidBodyComponentFromNative - Can't append rigid body component %s inside Lua VM.",
        rigidBodyComponent.GetName ().c_str ()
    );
}

void Actor::AppendRigidBodyComponentFromScript ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &physics
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &rigidBodyComponent = static_cast<RigidBodyComponent &> ( *component );

    if ( rigidBodyComponent.RegisterFromScript ( *this, physics ) )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendRigidBodyComponentFromScript - Can't register rigid body component %s.",
        rigidBodyComponent.GetName ().c_str ()
    );
}

void Actor::AppendScriptComponentFromNative ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    lua_pushvalue ( &vm, _appendComponentFromNativeIndex );
    lua_pushvalue ( &vm, -2 );

    // NOLINTNEXTLINE - downcast.
    auto &scriptComponent = static_cast<ScriptComponent &> ( *component );

    if ( !scriptComponent.RegisterFromNative ( vm, *this ) )
    {
        android_vulkan::LogWarning (
            "pbr::Actor::AppendScriptComponentFromNative - Can't register script component %s.",
            scriptComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendScriptComponentFromNative - Can't append script component %s inside Lua VM.",
        scriptComponent.GetName ().c_str ()
    );
}

void Actor::AppendScriptComponentFromScript ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &scriptComponent = static_cast<ScriptComponent &> ( *component );
    scriptComponent.RegisterFromScript ( *this );
}

void Actor::AppendSoundEmitterGlobalComponentFromNative ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    lua_pushvalue ( &vm, _appendComponentFromNativeIndex );
    lua_pushvalue ( &vm, -2 );

    // NOLINTNEXTLINE - downcast.
    auto &emitterComponent = static_cast<SoundEmitterGlobalComponent &> ( *component );

    if ( !emitterComponent.RegisterFromNative ( vm, *this ) )
    {
        android_vulkan::LogWarning (
            "pbr::Actor::AppendSoundEmitterGlobalComponentFromNative - Can't register sound emitter component %s.",
            emitterComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendSoundEmitterGlobalComponentFromNative - Can't append sound emitter component %s "
        "inside Lua VM.",
        emitterComponent.GetName ().c_str ()
    );
}

void Actor::AppendSoundEmitterGlobalComponentFromScript ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &emitterComponent = static_cast<SoundEmitterGlobalComponent &> ( *component );
    emitterComponent.RegisterFromScript ( *this );
}

void Actor::AppendSoundEmitterSpatialComponentFromNative ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    lua_pushvalue ( &vm, _appendComponentFromNativeIndex );
    lua_pushvalue ( &vm, -2 );

    // NOLINTNEXTLINE - downcast.
    auto &emitterComponent = static_cast<SoundEmitterSpatialComponent &> ( *component );

    if ( !emitterComponent.RegisterFromNative ( vm, *this ) )
    {
        android_vulkan::LogWarning (
            "pbr::Actor::AppendSoundEmitterSpatialComponentFromNative - Can't register sound emitter component %s.",
            emitterComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendSoundEmitterSpatialComponentFromNative - Can't append sound emitter component %s "
        "inside Lua VM.",
        emitterComponent.GetName ().c_str ()
    );
}

void Actor::AppendSoundEmitterSpatialComponentFromScript ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &emitterComponent = static_cast<SoundEmitterSpatialComponent &> ( *component );
    emitterComponent.RegisterFromScript ( *this );
}

void Actor::AppendStaticMeshComponentFromNative ( ComponentRef &component,
    ComponentList &renderable,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &transformable = static_cast<StaticMeshComponent &> ( *component );

    renderable.emplace_back ( transformable );
    _transformableComponents.emplace_back ( std::ref ( transformable ) );

    lua_pushvalue ( &vm, _appendComponentFromNativeIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !transformable.RegisterFromNative ( vm, *this ) )
    {
        android_vulkan::LogWarning (
            "pbr::Actor::AppendStaticMeshComponentFromNative - Can't register static mesh component %s.",
            transformable.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendStaticMeshComponentFromNative - Can't append static mesh component %s inside Lua VM.",
        transformable.GetName ().c_str ()
    );
}

void Actor::AppendStaticMeshComponentFromScript ( ComponentRef &component,
    ComponentList &renderable,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &transformable = static_cast<StaticMeshComponent &> ( *component );

    renderable.emplace_back ( transformable );
    _transformableComponents.emplace_back ( std::ref ( transformable ) );
    transformable.RegisterFromScript ( *this );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendTransformComponentFromNative ( ComponentRef &component,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &transformComponent = static_cast<TransformComponent &> ( *component );

    lua_pushvalue ( &vm, _appendComponentFromNativeIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !transformComponent.Register ( vm ) )
    {
        android_vulkan::LogWarning (
            "pbr::Actor::AppendTransformComponentFromNative - Can't register transform component %s.",
            transformComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning (
        "pbr::Actor::AppendTransformComponentFromNative - Can't append camera component %s inside Lua VM.",
        transformComponent.GetName ().c_str ()
    );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendTransformComponentFromScript ( ComponentRef &/*component*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // TODO
    AV_ASSERT ( false )
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendUnknownComponentFromNative ( ComponentRef &/*component*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &/*vm*/
) noexcept
{
    // IMPOSSIBLE
    AV_ASSERT ( false )
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendUnknownComponentFromScript ( ComponentRef &/*component*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/
) noexcept
{
    // IMPOSSIBLE
    AV_ASSERT ( false )
}

void Actor::DestroyRigidBodyComponent ( Component &component ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &rigidBodyComponent = static_cast<RigidBodyComponent &> ( component );
    rigidBodyComponent.Unregister ( _scene->GetPhysics () );
    RemoveComponent ( component );
}

void Actor::DestroyStaticMeshComponent ( Component &component ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    _scene->DetachRenderable ( static_cast<RenderableComponent const &> ( component ) );
    RemoveComponent ( component );
}

void Actor::DestroyComponentStub ( Component &/*component*/ ) noexcept
{
    // NOTHING
}

void Actor::RemoveComponent ( Component const &component ) noexcept
{
    auto const end = _components.cend ();

    auto const findResult = std::find_if ( _components.cbegin (),
        end,

        [ &component ] ( ComponentRef const &e ) noexcept -> bool {
            return e.get () == &component;
        }
    );

    AV_ASSERT ( findResult != end )
    _components.erase ( findResult );
}

int Actor::OnAppendComponent ( lua_State* state )
{
    auto &self = *static_cast<Actor*> ( lua_touserdata ( state, 1 ) );
    auto &component = *static_cast<Component*> ( lua_touserdata ( state, 2 ) );
    self.AppendComponent ( component.GetReference () );
    return 0;
}

int Actor::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::OnCreate - Stack is too small." );
        return 0;
    }

    ActorRef actor = std::make_shared<Actor> ( lua_tostring ( state, 1 ) );
    Actor* handle = actor.get ();
    _spawnActors.emplace ( handle, std::move ( actor ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int Actor::OnDestroy ( lua_State* state )
{
    auto const &self = *static_cast<Actor const*> ( lua_touserdata ( state, 1 ) );
    self._scene->RemoveActor ( self );
    return 0;
}

int Actor::OnGarbageCollected ( lua_State* state )
{
    _spawnActors.erase ( static_cast<Actor const*> ( lua_touserdata ( state, 1 ) ) );
    return 0;
}

int Actor::OnGetName ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::OnGetName - Stack is too small." );
        return 0;
    }

    auto const &self = *static_cast<Actor const*> ( lua_touserdata ( state, 1 ) );
    std::string const &n = self._name;
    lua_pushlstring ( state, n.c_str (), n.size () );
    return 1;
}

} // namespace pbr
