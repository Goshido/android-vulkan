#include <pbr/actor.h>
#include <pbr/camera_component.h>
#include <pbr/point_light_component.h>
#include <pbr/reflection_component.h>
#include <pbr/rigid_body_component.h>
#include <pbr/scene.h>
#include <pbr/script_component.h>
#include <pbr/script_engine.h>
#include <pbr/static_mesh_component.h>
#include <pbr/transform_component.h>
#include <guid_generator.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t ACTOR_DESC_FORMAT_VERSION = 1U;

//----------------------------------------------------------------------------------------------------------------------

class Actor::StaticInitializer final
{
    public:
        StaticInitializer () noexcept;

        StaticInitializer ( StaticInitializer const & ) = delete;
        StaticInitializer& operator = ( StaticInitializer const & ) = delete;

        StaticInitializer ( StaticInitializer && ) = delete;
        StaticInitializer& operator = ( StaticInitializer && ) = delete;

        ~StaticInitializer () = default;
};

Actor::StaticInitializer::StaticInitializer () noexcept
{
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::Camera ) ] = &Actor::DestroyComponentStub;
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::PointLight ) ] = &Actor::DestroyComponentStub;
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::Reflection ) ] = &Actor::DestroyComponentStub;
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::RigidBody ) ] = &Actor::DestroyRigidBodyComponent;
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::Script ) ] = &Actor::DestroyComponentStub;
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::StaticMesh ) ] = &Actor::DestroyStaticMeshComponent;
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::Transform ) ] = &Actor::DestroyComponentStub;
    Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::Unknown ) ] = &Actor::DestroyComponentStub;

    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::Camera ) ] = &Actor::AppendCameraComponent;
    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::PointLight ) ] = &Actor::AppendPointLightComponent;
    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::Reflection ) ] = &Actor::AppendReflectionComponent;
    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::RigidBody ) ] = &Actor::AppendRigidBodyComponent;
    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::Script ) ] = &Actor::AppendScriptComponent;
    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::StaticMesh ) ] = &Actor::AppendStaticMeshComponent;
    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::Transform ) ] = &Actor::AppendTransformComponent;
    Actor::_registerHandlers[ static_cast<size_t> ( ClassID::Unknown ) ] = &Actor::AppendUnknownComponent;
}

[[maybe_unused]] static Actor::StaticInitializer const g_StaticInitializer {};

//----------------------------------------------------------------------------------------------------------------------

int Actor::_appendComponentIndex = std::numeric_limits<int>::max ();
int Actor::_makeActorIndex = std::numeric_limits<int>::max ();
Actor::DestroyHander Actor::_destroyHandlers[ static_cast<size_t> ( ClassID::COUNT ) ] = {};
Actor::RegisterHander Actor::_registerHandlers[ static_cast<size_t> ( ClassID::COUNT ) ] = {};

Actor::Actor ( std::string &&name ) noexcept:
    _name ( std::move ( name ) )
{
    // NOTHING
}

Actor::Actor ( ActorDesc const &desc, uint8_t const* data ) noexcept
{
    // Sanity checks.
    static_assert ( sizeof ( ActorDesc::_localMatrix ) == sizeof ( desc._localMatrix ) );
    assert ( desc._formatVersion == ACTOR_DESC_FORMAT_VERSION );

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

std::string const& Actor::GetName () const noexcept
{
    return _name;
}

void Actor::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    for ( auto& component : _transformableComponents )
    {
        component.get ().OnTransform ( transformWorld );
    }
}

void Actor::RegisterComponents ( Scene &scene,
    ComponentList &freeTransferResource,
    ComponentList &renderable,
    android_vulkan::Physics &physics,
    lua_State &vm
) noexcept
{
    if ( !lua_checkstack ( &vm, 4 ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::RegisterComponents - Stack is too small." );
        return;
    }

    lua_pushvalue ( &vm, _makeActorIndex );
    lua_pushlightuserdata ( &vm, this );

    if ( lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK )
    {
        android_vulkan::LogWarning ( "pbr::Actor::RegisterComponents - Can't register actor inside Lua VM." );
        return;
    }

    _scene = &scene;

    for ( auto& component : _components )
    {
        RegisterHander const handler = _registerHandlers[ static_cast<size_t> ( component->GetClassID () ) ];

        // C++ calling method by pointer syntax.
        ( this->*handler ) ( component, freeTransferResource, renderable, physics, vm );
    }
}

bool Actor::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extentions[] =
    {
        {
            .name = "av_ActorDestroy",
            .func = &Actor::OnDestroy
        },
        {
            .name = "av_ActorGetName",
            .func = &Actor::OnGetName
        }
    };

    for ( auto const& extension : extentions )
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

    if ( !bind ( "AppendComponent", _appendComponentIndex ) )
        return false;

    return bind ( "MakeActor", _makeActorIndex );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendCameraComponent ( ComponentRef &component,
    ComponentList &/*freeTransferResource*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& cameraComponent = static_cast<CameraComponent&> ( *component );

    lua_pushvalue ( &vm, _appendComponentIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !cameraComponent.Register ( vm ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::AppendCameraComponent - Can't register camera component %s.",
            cameraComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning ( "pbr::Actor::AppendCameraComponent - Can't append camera component %s inside Lua VM.",
        cameraComponent.GetName ().c_str ()
    );
}

void Actor::AppendPointLightComponent ( ComponentRef &component,
    ComponentList &/*freeTransferResource*/,
    ComponentList &renderable,
    android_vulkan::Physics &/*physics*/,
    lua_State &/*vm*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& transformable = static_cast<PointLightComponent&> ( *component );

    renderable.emplace_back ( std::ref ( transformable ) );
    _transformableComponents.emplace_back ( std::ref ( transformable ) );
}

void Actor::AppendReflectionComponent ( ComponentRef &component,
    ComponentList &freeTransferResource,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &/*vm*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& reflection = static_cast<ReflectionComponent&> ( *component );

    freeTransferResource.emplace_back ( reflection );

    if ( !reflection.IsGlobalReflection () )
    {
        _transformableComponents.emplace_back ( std::ref ( reflection ) );
    }
}

void Actor::AppendRigidBodyComponent ( ComponentRef &component,
    ComponentList &/*freeTransferResource*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &physics,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& rigiBodyComponent = static_cast<RigidBodyComponent&> ( *component );

    lua_pushvalue ( &vm, _appendComponentIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !rigiBodyComponent.Register ( *this, physics, vm ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::AppendRigidBodyComponent - Can't register rigid body "
            "component %s.",
            rigiBodyComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning ( "pbr::Actor::AppendRigidBodyComponent - Can't append rigid body component %s "
        "inside Lua VM.",
        rigiBodyComponent.GetName ().c_str ()
    );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendScriptComponent ( ComponentRef &component,
    ComponentList &/*freeTransferResource*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    lua_pushvalue ( &vm, _appendComponentIndex );
    lua_pushvalue ( &vm, -2 );

    // NOLINTNEXTLINE - downcast.
    auto& scriptComponent = static_cast<ScriptComponent&> ( *component );

    if ( !scriptComponent.Register ( vm ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::AppendScriptComponent - Can't register script component %s.",
            scriptComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning ( "pbr::Actor::AppendScriptComponent - Can't append script component %s "
        "inside Lua VM.",
        scriptComponent.GetName ().c_str ()
    );
}

void Actor::AppendStaticMeshComponent ( ComponentRef &component,
    ComponentList &freeTransferResource,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& transformable = static_cast<StaticMeshComponent&> ( *component );

    freeTransferResource.emplace_back ( transformable );
    _transformableComponents.emplace_back ( std::ref ( transformable ) );

    lua_pushvalue ( &vm, _appendComponentIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !transformable.Register ( vm, *this ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::AppendStaticMeshComponent - Can't register static mesh component %s.",
            transformable.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning ( "pbr::Actor::AppendStaticMeshComponent - Can't append camera component %s inside "
        "Lua VM.",
        transformable.GetName ().c_str ()
    );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendTransformComponent ( ComponentRef &component,
    ComponentList &/*freeTransferResource*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &vm
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& transformComponent = static_cast<TransformComponent&> ( *component );

    lua_pushvalue ( &vm, _appendComponentIndex );
    lua_pushvalue ( &vm, -2 );

    if ( !transformComponent.Register ( vm ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::AppendTransformComponent - Can't register transform component %s.",
            transformComponent.GetName ().c_str ()
        );

        lua_pop ( &vm, 2 );
        return;
    }

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
        return;

    android_vulkan::LogWarning ( "pbr::Actor::AppendTransformComponent - Can't append camera component %s inside "
        "Lua VM.",
        transformComponent.GetName ().c_str ()
    );
}

// NOLINTNEXTLINE - can be made static.
void Actor::AppendUnknownComponent ( ComponentRef &/*component*/,
    ComponentList &/*freeTransferResource*/,
    ComponentList &/*renderable*/,
    android_vulkan::Physics &/*physics*/,
    lua_State &/*vm*/
) noexcept
{
    // IMPOSSIBLE
    assert ( false );
}

void Actor::DestroyRigidBodyComponent ( Component &component ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& rigidBodyComponent = static_cast<RigidBodyComponent&> ( component );
    rigidBodyComponent.Unregister ( _scene->GetPhysics () );
    RemoveComponent ( component );
}

void Actor::DestroyStaticMeshComponent ( Component &component ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    _scene->DetachRenderable ( static_cast<RenderableComponent const&> ( component ) );
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

    assert ( findResult != end );
    _components.erase ( findResult );
}

int Actor::OnDestroy ( lua_State* state )
{
    auto const& self = *static_cast<Actor const*> ( lua_touserdata ( state, 1 ) );
    self._scene->RemoveActor ( self );
    return 0;
}

int Actor::OnGetName ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::Actor::OnGetName - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Actor const*> ( lua_touserdata ( state, 1 ) );
    std::string const& n = self._name;
    lua_pushlstring ( state, n.c_str (), n.size () );
    return 1;
}

} // namespace pbr
