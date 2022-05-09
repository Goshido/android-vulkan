#include <pbr/rigid_body_component.h>
#include <pbr/actor.h>
#include <pbr/coordinate_system.h>
#include <pbr/script_engine.h>
#include <pbr/scriptable_gxvec3.h>
#include <guid_generator.h>
#include <physics.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

int RigidBodyComponent::_registerRigidBodyComponentIndex = std::numeric_limits<int>::max ();

RigidBodyComponent::RigidBodyComponent ( android_vulkan::ShapeRef &shape ) noexcept:
    Component ( ClassID::RigidBody, android_vulkan::GUID::GenerateAsString ( "RigidBody" ) ),
    _actor ( nullptr ),
    _rigidBody ( std::make_shared<android_vulkan::RigidBody> () )
{
    Setup ( shape );
}

RigidBodyComponent::RigidBodyComponent ( android_vulkan::ShapeRef &shape, std::string &&name ) noexcept:
    Component ( ClassID::RigidBody, std::move ( name ) ),
    _actor ( nullptr ),
    _rigidBody ( std::make_shared<android_vulkan::RigidBody> () )
{
    Setup ( shape );
}

android_vulkan::RigidBodyRef& RigidBodyComponent::GetRigidBody () noexcept
{
    return _rigidBody;
}

bool RigidBodyComponent::Register ( Actor &actor, android_vulkan::Physics &physics, lua_State &vm ) noexcept
{
    _actor = &actor;

    if ( !physics.AddRigidBody ( _rigidBody ) )
        return false;

    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::Register - Stack too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerRigidBodyComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

bool RigidBodyComponent::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) )
    {
        android_vulkan::LogError ( "pbr::RigidBodyComponent::Init - Stack too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterRigidBodyComponent" ) != LUA_TFUNCTION )
    {
        android_vulkan::LogError ( "pbr::RigidBodyComponent::Init - Can't find register function." );
        return false;
    }

    _registerRigidBodyComponentIndex = lua_gettop ( &vm );

    constexpr luaL_Reg const extentions[] =
    {
        {
            .name = "av_RigidBodyComponentCreate",
            .func = &RigidBodyComponent::OnCreate
        },

        {
            .name = "av_RigidBodyComponentGetLocation",
            .func = &RigidBodyComponent::OnGetLocation
        }
    };

    for ( auto const& extension : extentions )
        lua_register ( &vm, extension.name, extension.func );

    return true;
}

void RigidBodyComponent::Setup ( android_vulkan::ShapeRef &shape ) noexcept
{
    android_vulkan::RigidBody& body = *_rigidBody;
    body.SetShape ( shape, true );
    body.SetContext ( this );
    body.SetTransformUpdateHandler ( &RigidBodyComponent::OnTransformUpdate );
}

int RigidBodyComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int RigidBodyComponent::OnGetLocation ( lua_State* state )
{
    auto const& self = *static_cast<RigidBodyComponent const*> ( lua_touserdata ( state, 1 ) );
    ScriptableGXVec3::Extract ( state, 2 ) = self._rigidBody->GetLocation ();
    return 0;
}

void RigidBodyComponent::OnTransformUpdate ( android_vulkan::RigidBody::Context context,
    GXVec3 const &location,
    GXQuat const &rotation
) noexcept
{
    auto& component = *static_cast<RigidBodyComponent*> ( context );

    GXVec3 origin {};
    origin.Multiply ( location, UNITS_IN_METER );

    GXMat4 transform {};
    transform.FromFast ( rotation, origin );

    component._actor->OnTransform ( transform );
}

} // namespace pbr
