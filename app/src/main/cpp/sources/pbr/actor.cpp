#include <pbr/actor.h>
#include <pbr/script_component.h>
#include <pbr/script_engine.h>
#include <pbr/rigid_body_component.h>
#include <guid_generator.h>
#include <logger.h>


namespace pbr {

int Actor::_appendComponentIndex = std::numeric_limits<int>::max ();
int Actor::_makeActorIndex = std::numeric_limits<int>::max ();

Actor::Actor () noexcept:
    _name ( android_vulkan::GUID::GenerateAsString ( "Actor" ) )
{
    // NOTHING
}

Actor::Actor ( std::string &&name ) noexcept:
    _name ( std::move ( name ) )
{
    // NOTHING
}

void Actor::AppendComponent ( ComponentRef &component ) noexcept
{
    _components.push_back ( component );
}

std::string const& Actor::GetName () const noexcept
{
    return _name;
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

    for ( auto& component : _components )
    {
        ClassID const classID = component->GetClassID ();

        if ( classID == ClassID::Reflection | classID == ClassID::StaticMesh )
        {
            freeTransferResource.emplace_back ( std::ref ( component ) );
            continue;
        }

        if ( classID == ClassID::RigidBody )
        {
            // NOLINTNEXTLINE - downcast.
            auto& rigiBodyComponent = static_cast<RigidBodyComponent&> ( *component );

            if ( !rigiBodyComponent.Register ( physics ) )
            {
                android_vulkan::LogWarning ( "pbr::Actor::RegisterComponents - Can't register rigid body "
                    "component %s.",
                    rigiBodyComponent.GetName ().c_str ()
                );
            }

            continue;
        }

        if ( classID == ClassID::PointLight )
        {
            renderable.emplace_back ( std::ref ( component ) );
            continue;
        }

        if ( classID == ClassID::Script )
        {
            lua_pushvalue ( &vm, _appendComponentIndex );
            lua_pushvalue ( &vm, -2 );

            // NOLINTNEXTLINE - downcast.
            auto& scriptComponent = static_cast<ScriptComponent&> ( *component );

            if ( !scriptComponent.Register ( vm ) )
            {
                android_vulkan::LogWarning ( "pbr::Actor::RegisterComponents - Can't register script component %s.",
                    scriptComponent.GetName ().c_str ()
                );

                continue;
            }

            if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
                continue;

            android_vulkan::LogWarning ( "pbr::Actor::RegisterComponents - Can't register script component "
                "inside Lua VM."
            );
        }
    }
}

bool Actor::Register ( lua_State &vm ) noexcept
{
    lua_register ( &vm, "av_ActorGetName", &Actor::OnGetName );

    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::Actor::Register - Stack is too small." );
        return false;
    }

    auto bind = [ & ] ( char const* function, int &ind ) noexcept -> bool {
        if ( lua_getglobal ( &vm, function ) == LUA_TFUNCTION )
        {
            ind = lua_gettop ( &vm );
            return true;
        }

        android_vulkan::LogError ( "pbr::Actor::Register - Can't find %s function.", function );
        return false;
    };

    if ( !bind ( "AppendComponent", _appendComponentIndex ) )
        return false;

    return bind ( "MakeActor", _makeActorIndex );
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
