#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/scriptable_text_ui_element.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

ScriptableTextUIElement::ScriptableTextUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    std::u32string &&text
) noexcept:
    _text ( true, parent, std::move ( text ) )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableTextUIElement::ScriptableTextUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterTextUIElement" ) == LUA_TFUNCTION; !success ) [[unlikely]]
    {
        android_vulkan::LogError (
            "pbr::ScriptableTextUIElement::ScriptableTextUIElement - Can't find register function."
        );

        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; success ) [[likely]]
        return;

    android_vulkan::LogWarning (
        "pbr::ScriptableTextUIElement::ScriptableTextUIElement - Can't append element inside Lua VM."
    );
}

UIElement &ScriptableTextUIElement::GetElement () noexcept
{
    return _text;
}

UIElement const &ScriptableTextUIElement::GetElement () const noexcept
{
    return _text;
}

void ScriptableTextUIElement::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_TextUIElementSetColorHSV",
            .func = &ScriptableTextUIElement::OnSetColorHSV
        },
        {
            .name = "av_TextUIElementSetColorRGB",
            .func = &ScriptableTextUIElement::OnSetColorRGB
        },
        {
            .name = "av_TextUIElementSetText",
            .func = &ScriptableTextUIElement::OnSetText
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

int ScriptableTextUIElement::OnSetColorHSV ( lua_State* state )
{
    auto const h = static_cast<float> ( lua_tonumber ( state, 2 ) );
    auto const s = static_cast<float> ( lua_tonumber ( state, 3 ) );
    auto const v = static_cast<float> ( lua_tonumber ( state, 4 ) );
    auto const a = static_cast<float> ( lua_tonumber ( state, 5 ) );

    auto &self = *static_cast<ScriptableTextUIElement*> ( lua_touserdata ( state, 1 ) );
    self._text.SetColor ( GXColorRGB ( GXColorHSV ( h, s, v, a ) ) );
    return 0;
}

int ScriptableTextUIElement::OnSetColorRGB ( lua_State* state )
{
    auto const r = static_cast<GXUByte> ( lua_tonumber ( state, 2 ) );
    auto const g = static_cast<GXUByte> ( lua_tonumber ( state, 3 ) );
    auto const b = static_cast<GXUByte> ( lua_tonumber ( state, 4 ) );
    auto const a = static_cast<GXUByte> ( lua_tonumber ( state, 5 ) );

    auto &self = *static_cast<ScriptableTextUIElement*> ( lua_touserdata ( state, 1 ) );
    self._text.SetColor ( GXColorRGB ( r, g, b, a ) );
    return 0;
}

int ScriptableTextUIElement::OnSetText ( lua_State* state )
{
    auto &self = *static_cast<ScriptableTextUIElement*> ( lua_touserdata ( state, 1 ) );
    self._text.SetText ( lua_tostring ( state, 2 ) );
    return 0;
}

} // namespace pbr
