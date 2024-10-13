#include <logger.hpp>
#include <pbr/scriptable_div_ui_element.hpp>


namespace pbr {

ScriptableDIVUIElement::ScriptableDIVUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    CSSComputedValues &&css
) noexcept:
    _div ( parent, std::move ( css ) )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableDIVUIElement::ScriptableDIVUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterDIVUIElement" ) == LUA_TFUNCTION; !success ) [[unlikely]]
    {
        android_vulkan::LogError (
            "pbr::ScriptableDIVUIElement::ScriptableDIVUIElement - Can't find register function."
        );

        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; success ) [[likely]]
        return;

    android_vulkan::LogWarning (
        "pbr::ScriptableDIVUIElement::ScriptableDIVUIElement - Can't append element inside Lua VM."
    );
}

UIElement &ScriptableDIVUIElement::GetElement () noexcept
{
    return _div;
}

UIElement const &ScriptableDIVUIElement::GetElement () const noexcept
{
    return _div;
}

bool ScriptableDIVUIElement::AppendChildElement ( lua_State &vm,
    int errorHandlerIdx,
    int appendChildElementIdx,
    UIElement &element
) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableDIVUIElement::AppendChildElement - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, appendChildElementIdx );
    lua_pushvalue ( &vm, -3 );
    lua_rotate ( &vm, -3, -1 );

    if ( lua_pcall ( &vm, 2, 0, errorHandlerIdx ) == LUA_OK ) [[likely]]
    {
        _div.AppendChildElement ( element );
        return true;
    }

    android_vulkan::LogWarning (
        "pbr::ScriptableDIVUIElement::AppendChildElement - Can't append child element inside Lua VM."
    );

    return false;
}

} // namespace pbr
