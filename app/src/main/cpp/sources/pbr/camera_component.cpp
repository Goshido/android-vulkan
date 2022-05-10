#include <pbr/camera_component.h>
#include <pbr/script_engine.h>
#include <pbr/scriptable_gxmat4.h>
#include <guid_generator.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

int CameraComponent::_registerCameraComponentIndex = std::numeric_limits<int>::max ();

// NOLINTNEXTLINE - uninitialized fields
CameraComponent::CameraComponent () noexcept:
    Component ( ClassID::Camera, android_vulkan::GUID::GenerateAsString ( "Camera" ) )
{
    _local.Identity ();
    _projection.Identity ();
}

// NOLINTNEXTLINE - uninitialized fields
CameraComponent::CameraComponent ( std::string &&name ) noexcept:
    Component ( ClassID::Camera, std::move ( name ) )
{
    _local.Identity ();
    _projection.Identity ();
}

GXMat4 const& CameraComponent::GetLocalMatrix () const noexcept
{
    return _local;
}

GXMat4 const& CameraComponent::GetProjectionMatrix () const noexcept
{
    return _projection;
}

void CameraComponent::SetLocal ( GXMat4 const &local ) noexcept
{
    _local = local;
}

void CameraComponent::SetProjection ( float fieldOfViewRadians, float aspectRatio, float zNear, float zFar ) noexcept
{
    _projection.Perspective ( fieldOfViewRadians, aspectRatio, zNear, zFar );
}

bool CameraComponent::Register ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::CameraComponent::Register - Stack too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerCameraComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

bool CameraComponent::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) )
    {
        android_vulkan::LogError ( "pbr::CameraComponent::Init - Stack too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterCameraComponent" ) != LUA_TFUNCTION )
    {
        android_vulkan::LogError ( "pbr::CameraComponent::Init - Can't find register function." );
        return false;
    }

    _registerCameraComponentIndex = lua_gettop ( &vm );

    constexpr luaL_Reg const extentions[] =
    {
        {
            .name = "av_CameraComponentCreate",
            .func = &CameraComponent::OnCreate
        },

        {
            .name = "av_CameraComponentSetLocal",
            .func = &CameraComponent::OnSetLocal
        },

        {
            .name = "av_CameraComponentSetProjection",
            .func = &CameraComponent::OnSetProjection
        }
    };

    for ( auto const& extension : extentions )
        lua_register ( &vm, extension.name, extension.func );

    return true;
}

int CameraComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int CameraComponent::OnSetLocal ( lua_State* state )
{
    auto& self = *static_cast<CameraComponent*> ( lua_touserdata ( state, 1 ) );
    self._local = ScriptableGXMat4::Extract ( state, 2 );
    return 0;
}

int CameraComponent::OnSetProjection ( lua_State* state )
{
    auto& self = *static_cast<CameraComponent*> ( lua_touserdata ( state, 1 ) );

    self._projection.Perspective ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) ),
        static_cast<float> ( lua_tonumber ( state, 5 ) )
    );

    return 0;
}

} // namespace pbr
