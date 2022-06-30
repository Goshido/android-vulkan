#include <pbr/actor.h>
#include <pbr/camera_component.h>
#include <pbr/point_light_component.h>
#include <pbr/reflection_component.h>
#include <pbr/rigid_body_component.h>
#include <pbr/script_component.h>
#include <pbr/script_engine.h>
#include <pbr/static_mesh_component.h>
#include <guid_generator.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t ACTOR_DESC_FORMAT_VERSION = 1U;


//----------------------------------------------------------------------------------------------------------------------

int Actor::_appendComponentIndex = std::numeric_limits<int>::max ();
int Actor::_makeActorIndex = std::numeric_limits<int>::max ();
std::unordered_map<ClassID, Actor::RegisterHander> Actor::_registerHandlers {};

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

void Actor::RegisterComponents ( ComponentList &freeTransferResource,
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

    auto const end = _registerHandlers.end ();

    for ( auto& component : _components )
    {
        auto findResult = _registerHandlers.find ( component->GetClassID () );

        if ( findResult == end )
            continue;

        RegisterHander const handler = findResult->second;

        // C++ calling method by pointer syntax.
        ( this->*handler ) ( component, freeTransferResource, renderable, physics, vm );
    }
}

bool Actor::Init ( lua_State &vm ) noexcept
{
    _registerHandlers.emplace ( std::pair ( ClassID::Camera, &Actor::AppendCameraComponent ) );
    _registerHandlers.emplace ( std::pair ( ClassID::PointLight, &Actor::AppendPointLightComponent ) );
    _registerHandlers.emplace ( std::pair ( ClassID::Reflection, &Actor::AppendReflectionComponent ) );
    _registerHandlers.emplace ( std::pair ( ClassID::RigidBody, &Actor::AppendRigidBodyComponent ) );
    _registerHandlers.emplace ( std::pair ( ClassID::Script, &Actor::AppendScriptComponent ) );
    _registerHandlers.emplace ( std::pair ( ClassID::StaticMesh, &Actor::AppendStaticMeshComponent ) );

    lua_register ( &vm, "av_ActorGetName", &Actor::OnGetName );

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

void Actor::Destroy () noexcept
{
    _registerHandlers.clear ();
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

    renderable.emplace_back ( std::ref ( component ) );
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

    freeTransferResource.emplace_back ( std::ref ( component ) );

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
    lua_State &/*vm*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& transformable = static_cast<StaticMeshComponent&> ( *component );

    freeTransferResource.emplace_back ( std::ref ( component ) );
    _transformableComponents.emplace_back ( std::ref ( transformable ) );
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
