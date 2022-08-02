#include <pbr/rigid_body_component.h>
#include <pbr/actor.h>
#include <pbr/coordinate_system.h>
#include <pbr/script_engine.h>
#include <pbr/scriptable_gxvec3.h>
#include <guid_generator.h>
#include <physics.h>
#include <shape_box.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <cassert>
#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t RIGID_BODY_COMPONENT_DESC_FORMAT_VERSION = 1U;
[[maybe_unused]] constexpr static uint32_t BOX_SHAPE_DESC_FORMAT_VERSION = 1U;

//----------------------------------------------------------------------------------------------------------------------

int RigidBodyComponent::_registerRigidBodyComponentIndex = std::numeric_limits<int>::max ();

RigidBodyComponent::RigidBodyComponent ( size_t &dataRead,
    RigidBodyComponentDesc const &desc,
    uint8_t const* data
) noexcept:
    Component ( ClassID::RigidBody ),
    _actor ( nullptr ),
    _rigidBody ( std::make_shared<android_vulkan::RigidBody> () )
{
    // Sanity checks.
    assert ( desc._formatVersion == RIGID_BODY_COMPONENT_DESC_FORMAT_VERSION );

    _name = reinterpret_cast<char const*> ( data + desc._name );

    static_assert ( sizeof ( GXMat4 ) == sizeof ( desc._localMatrix ) );
    auto const& m = reinterpret_cast<GXMat4 const&> ( desc._localMatrix );

    // NOLINTNEXTLINE - downcast.
    auto& body = static_cast<android_vulkan::RigidBody&> ( *_rigidBody );
    body.SetLocation ( *reinterpret_cast<GXVec3 const*> ( &m._m[ 3U ][ 0U ] ), true );

    GXQuat r {};
    r.FromFast ( m );
    body.SetRotation ( r, true );

    switch ( desc._type )
    {
        case eRigidBodyTypeDesc::Dynamic:
            body.DisableKinematic ( true );
        break;

        case eRigidBodyTypeDesc::Kinematic:
            body.EnableKinematic ();
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    auto const* base = reinterpret_cast<uint8_t const*> ( &desc );
    auto const& shapeDesc = *reinterpret_cast<ShapeDesc const*> ( base + sizeof ( RigidBodyComponentDesc ) );

    assert ( shapeDesc._type == eShapeTypeDesc::Box );

    // NOLINTNEXTLINE - downcast.
    auto const& boxDesc = static_cast<ShapeBoxDesc const&> ( shapeDesc );

    // Sanity checks.
    assert ( boxDesc._formatVersion == BOX_SHAPE_DESC_FORMAT_VERSION );

    android_vulkan::ShapeRef shape = std::make_shared<android_vulkan::ShapeBox> ( boxDesc._dimensions[ 0U ],
        boxDesc._dimensions[ 1U ],
        boxDesc._dimensions[ 2U ]
    );

    // NOLINTNEXTLINE - downcast.
    auto& boxShape = static_cast<android_vulkan::ShapeBox&> ( *shape );
    boxShape.SetCollisionGroups ( boxDesc._collisionGroups );
    boxShape.SetFriction ( boxDesc._friction );
    boxShape.SetRestitution ( boxDesc._restitution );

    body.SetMass ( boxDesc._mass, true );
    dataRead += sizeof ( ShapeBoxDesc );

    Setup ( shape );
}

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

bool RigidBodyComponent::Register ( Actor &actor, android_vulkan::Physics &physics, lua_State &vm ) noexcept
{
    _actor = &actor;

    if ( !physics.AddRigidBody ( _rigidBody ) )
        return false;

    if ( !lua_checkstack ( &vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::Register - Stack too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerRigidBodyComponentIndex );
    lua_pushlightuserdata ( &vm, this );
    lua_pushlightuserdata ( &vm, _rigidBody.get () );

    return lua_pcall ( &vm, 2, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
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
            .name = "av_RigidBodyComponentAddForce",
            .func = &RigidBodyComponent::OnAddForce
        },
        {
            .name = "av_RigidBodyComponentCreate",
            .func = &RigidBodyComponent::OnCreate
        },
        {
            .name = "av_RigidBodyComponentGetLocation",
            .func = &RigidBodyComponent::OnGetLocation
        },
        {
            .name = "av_RigidBodyComponentSetLocation",
            .func = &RigidBodyComponent::OnSetLocation
        },
        {
            .name = "av_RigidBodyComponentGetVelocityLinear",
            .func = &RigidBodyComponent::OnGetVelocityLinear
        },
        {
            .name = "av_RigidBodyComponentSetVelocityLinear",
            .func = &RigidBodyComponent::OnSetVelocityLinear
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

int RigidBodyComponent::OnAddForce ( lua_State* state )
{
    auto& self = *static_cast<RigidBodyComponent*> ( lua_touserdata ( state, 1 ) );

    self._rigidBody->AddForce ( ScriptableGXVec3::Extract ( state, 2 ),
        ScriptableGXVec3::Extract ( state, 3 ),
        static_cast<bool> ( lua_toboolean ( state, 4 ) )
    );

    return 0;
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

int RigidBodyComponent::OnSetLocation ( lua_State* state )
{
    auto& self = *static_cast<RigidBodyComponent*> ( lua_touserdata ( state, 1 ) );
    self._rigidBody->SetLocation ( ScriptableGXVec3::Extract ( state, 2 ), true );
    return 0;
}

int RigidBodyComponent::OnGetVelocityLinear ( lua_State* state )
{
    auto const& self = *static_cast<RigidBodyComponent const*> ( lua_touserdata ( state, 1 ) );
    ScriptableGXVec3::Extract ( state, 2 ) = self._rigidBody->GetVelocityLinear ();
    return 0;
}

int RigidBodyComponent::OnSetVelocityLinear ( lua_State* state )
{
    auto& self = *static_cast<RigidBodyComponent*> ( lua_touserdata ( state, 1 ) );

    self._rigidBody->SetVelocityLinear ( ScriptableGXVec3::Extract ( state, 2 ),
        static_cast<bool> ( lua_toboolean ( state, 3 ) )
    );

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
