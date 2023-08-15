#include <pbr/camera_component.hpp>
#include <pbr/script_engine.hpp>
#include <pbr/scriptable_gxmat4.hpp>
#include <av_assert.hpp>
#include <guid_generator.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

[[maybe_unused]] constexpr uint32_t CAMERA_COMPONENT_DESC_FORMAT_VERSION = 1U;

constexpr float DEFAULT_FIELD_OF_VIEW = 60.0F;
constexpr float DEFAULT_Z_NEAR = 1.0e-1F;
constexpr float DEFAULT_Z_FAR = 1.0e+4F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

int CameraComponent::_registerCameraComponentIndex = std::numeric_limits<int>::max ();

CameraComponent::CameraComponent () noexcept:
    Component ( ClassID::Camera, android_vulkan::GUID::GenerateAsString ( "Camera" ) ),
    _zNear ( DEFAULT_Z_NEAR ),
    _zFar ( DEFAULT_Z_FAR ),
    _fieldOfViewRadians ( GXDegToRad ( DEFAULT_FIELD_OF_VIEW ) )
{
    _local.Identity ();
    _projection.Identity ();
}

// NOLINTNEXTLINE - uninitialized fields
CameraComponent::CameraComponent ( CameraComponentDesc const &desc, uint8_t const* data ) noexcept:
    Component ( ClassID::Camera )
{
    // Sanity checks.
    static_assert ( sizeof ( desc._localMatrix ) == sizeof ( GXMat4 ) );
    AV_ASSERT ( desc._formatVersion == CAMERA_COMPONENT_DESC_FORMAT_VERSION )

    _name = reinterpret_cast<char const*> ( data + desc._name );
    std::memcpy ( _local._data, &desc._localMatrix, sizeof ( _local ) );

    SetProjection ( desc._fieldOfViewRadians, 1.0F, desc._zNear, desc._zFar );
}

CameraComponent::CameraComponent ( std::string &&name ) noexcept:
    Component ( ClassID::Camera, std::move ( name ) ),
    _zNear ( DEFAULT_Z_NEAR ),
    _zFar ( DEFAULT_Z_FAR ),
    _fieldOfViewRadians ( GXDegToRad ( DEFAULT_FIELD_OF_VIEW ) )
{
    _local.Identity ();
    _projection.Identity ();
}

GXMat4 const &CameraComponent::GetLocalMatrix () const noexcept
{
    return _local;
}

GXMat4 const &CameraComponent::GetProjectionMatrix () const noexcept
{
    return _projection;
}

void CameraComponent::SetAspectRatio ( float aspectRatio ) noexcept
{
    _projection.Perspective ( _fieldOfViewRadians, aspectRatio, _zNear, _zFar );
}

void CameraComponent::SetLocal ( GXMat4 const &local ) noexcept
{
    _local = local;
}

void CameraComponent::SetProjection ( float fieldOfViewRadians, float aspectRatio, float zNear, float zFar ) noexcept
{
    _projection.Perspective ( fieldOfViewRadians, aspectRatio, zNear, zFar );
    _zNear = zNear;
    _zFar = zFar;
    _fieldOfViewRadians = fieldOfViewRadians;
}

bool CameraComponent::Register ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::CameraComponent::Register - Stack is too small." );
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
        android_vulkan::LogError ( "pbr::CameraComponent::Init - Stack is too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterCameraComponent" ) != LUA_TFUNCTION )
    {
        android_vulkan::LogError ( "pbr::CameraComponent::Init - Can't find register function." );
        return false;
    }

    _registerCameraComponentIndex = lua_gettop ( &vm );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_CameraComponentCreate",
            .func = &CameraComponent::OnCreate
        },

        {
            .name = "av_CameraComponentSetAspectRatio",
            .func = &CameraComponent::OnSetAspectRatio
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

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    return true;
}

ComponentRef &CameraComponent::GetReference () noexcept
{
    // TODO
    static ComponentRef dummy {};
    return dummy;
}

int CameraComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int CameraComponent::OnSetAspectRatio ( lua_State* state )
{
    auto &self = *static_cast<CameraComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetAspectRatio ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int CameraComponent::OnSetLocal ( lua_State* state )
{
    auto &self = *static_cast<CameraComponent*> ( lua_touserdata ( state, 1 ) );
    self._local = ScriptableGXMat4::Extract ( state, 2 );
    return 0;
}

int CameraComponent::OnSetProjection ( lua_State* state )
{
    auto &self = *static_cast<CameraComponent*> ( lua_touserdata ( state, 1 ) );

    self.SetProjection ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) ),
        static_cast<float> ( lua_tonumber ( state, 5 ) )
    );

    return 0;
}

} // namespace pbr
