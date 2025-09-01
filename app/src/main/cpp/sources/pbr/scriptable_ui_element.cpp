#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/scriptable_ui_element.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

ScriptableUIElement::Storage ScriptableUIElement::_uiElements {};

void ScriptableUIElement::AppendElement ( ScriptableUIElement &element ) noexcept
{
    _uiElements.emplace ( &element, std::unique_ptr<ScriptableUIElement> ( &element ) );
}

void ScriptableUIElement::InitCommon ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_UIElementCollectGarbage",
            .func = &ScriptableUIElement::OnGarbageCollected
        },
        {
            .name = "av_UIElementHide",
            .func = &ScriptableUIElement::OnHide
        },
        {
            .name = "av_UIElementIsVisible",
            .func = &ScriptableUIElement::OnIsVisible
        },
        {
            .name = "av_UIElementShow",
            .func = &ScriptableUIElement::OnShow
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void ScriptableUIElement::Destroy () noexcept
{
    if ( !_uiElements.empty () )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableUIElement::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _uiElements.clear ();
}

int ScriptableUIElement::OnGarbageCollected ( lua_State* state )
{
    auto const* element = static_cast<ScriptableUIElement const*> ( lua_touserdata ( state, 1 ) );

    if ( auto const findResult = _uiElements.find ( element ); findResult != _uiElements.cend () )
    {
        _uiElements.erase ( findResult );
        return 0;
    }

    android_vulkan::LogWarning ( "pbr::ScriptableUIElement::OnGarbageCollected - Can't find element." );
    AV_ASSERT ( false )
    return 0;
}

int ScriptableUIElement::OnHide ( lua_State* state )
{
    auto &item = *static_cast<ScriptableUIElement*> ( lua_touserdata ( state, 1 ) );
    item.GetElement ().Hide ();
    return 0;
}

int ScriptableUIElement::OnIsVisible ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableUIElement::OnIsVisible - Stack is too small." );
        return 0;
    }

    auto const &item = *static_cast<ScriptableUIElement const*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, item.GetElement ().IsVisible () );
    return 1;
}

int ScriptableUIElement::OnShow ( lua_State* state )
{
    auto &item = *static_cast<ScriptableUIElement*> ( lua_touserdata ( state, 1 ) );
    item.GetElement ().Show ();
    return 0;
}

} // namespace pbr
