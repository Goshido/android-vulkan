#include <pbr/transform_component.hpp>
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

[[maybe_unused]] constexpr uint32_t TRANSFORM_COMPONENT_DESC_FORMAT_VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

int TransformComponent::_registerTransformComponentIndex = std::numeric_limits<int>::max ();

TransformComponent::TransformComponent () noexcept:
    Component ( ClassID::Transform, android_vulkan::GUID::GenerateAsString ( "Transform" ) )
{
    _local.Identity ();
}

TransformComponent::TransformComponent ( TransformComponentDesc const &desc, uint8_t const* data ) noexcept:
    Component ( ClassID::Transform )
{
    // Sanity checks.
    static_assert ( sizeof ( desc._localMatrix ) == sizeof ( GXMat4 ) );
    AV_ASSERT ( desc._formatVersion == TRANSFORM_COMPONENT_DESC_FORMAT_VERSION )

    _name = reinterpret_cast<char const*> ( data + desc._name );
    std::memcpy ( _local._data, &desc._localMatrix, sizeof ( _local ) );
}

bool TransformComponent::Register ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 2 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::TransformComponent::Register - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerTransformComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

bool TransformComponent::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::TransformComponent::Init - Stack is too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterTransformComponent" ) != LUA_TFUNCTION ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::TransformComponent::Init - Can't find register function." );
        return false;
    }

    _registerTransformComponentIndex = lua_gettop ( &vm );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_TransformComponentCreate",
            .func = &TransformComponent::OnCreate
        },

        {
            .name = "av_TransformComponentGetTransform",
            .func = &TransformComponent::OnGetTransform
        }
    };

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    return true;
}

ComponentRef &TransformComponent::GetReference () noexcept
{
    // TODO
    static ComponentRef dummy {};
    return dummy;
}

int TransformComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int TransformComponent::OnGetTransform ( lua_State* state )
{
    auto const &self = *static_cast<TransformComponent const*> ( lua_touserdata ( state, 1 ) );
    ScriptableGXMat4::Extract ( state, 2 ) = self._local;
    return 0;
}

} // namespace pbr
