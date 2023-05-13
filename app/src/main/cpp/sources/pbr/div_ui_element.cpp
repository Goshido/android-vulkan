#include <pbr/div_ui_element.h>
#include <pbr/script_engine.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

DIVUIElement::DIVUIElement ( bool &success, lua_State &vm, CSSComputedValues const &css ) noexcept:
    UIElement ( css._display != DisplayProperty::eValue::None ),
    _css ( css )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterDIVUIElement" ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK; !success )
    {
        android_vulkan::LogWarning ( "pbr::DIVUIElement::DIVUIElement - Can't append element inside Lua VM." );
    }
}

bool DIVUIElement::AppendChildElement ( lua_State &vm,
    int appendChildElementIdx,
    std::unique_ptr<UIElement> &&element
) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::AppendChildElement - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, appendChildElementIdx );
    lua_pushvalue ( &vm, -3 );
    lua_rotate ( &vm, -3, -1 );

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
    {
        _childs.emplace_back ( std::move ( element ) );
        return true;
    }

    android_vulkan::LogWarning ( "pbr::DIVUIElement::AppendChildElement - Can't append child element inside Lua VM." );
    return false;
}

void DIVUIElement::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_DIVUIElementCollectGarbage",
            .func = &DIVUIElement::OnGarbageCollected
        },
        {
            .name = "av_DIVUIElementHide",
            .func = &DIVUIElement::OnHide
        },
        {
            .name = "av_DIVUIElementShow",
            .func = &DIVUIElement::OnShow
        }
    };

    for ( auto const& extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void DIVUIElement::ApplyLayout () noexcept
{
    // TODO
}

void DIVUIElement::Render () noexcept
{
    // TODO
}

int DIVUIElement::OnGarbageCollected ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int DIVUIElement::OnHide ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int DIVUIElement::OnShow ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
