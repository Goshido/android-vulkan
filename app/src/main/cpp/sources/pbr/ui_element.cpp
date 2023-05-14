#include <pbr/ui_element.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

void UIElement::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {

        {
            .name = "av_UIElementHide",
            .func = &UIElement::OnHide
        },
        {
            .name = "av_UIElementIsVisible",
            .func = &UIElement::OnIsVisible
        },
        {
            .name = "av_UIElementShow",
            .func = &UIElement::OnShow
        }
    };

    for ( auto const& extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

UIElement::UIElement ( bool visible ) noexcept:
    _visible ( visible )
{
    // NOTHING
}

int UIElement::OnHide ( lua_State* state )
{
    auto& item = *static_cast<UIElement*> ( lua_touserdata ( state, 1 ) );
    item._visible = false;
    return 0;
}

int UIElement::OnIsVisible ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::UIElement::OnIsVisible - Stack is too small." );
        return 0;
    }

    auto const& item = *static_cast<UIElement const*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, item._visible );
    return 1;
}

int UIElement::OnShow ( lua_State* state )
{
    auto& item = *static_cast<UIElement*> ( lua_touserdata ( state, 1 ) );
    item._visible = true;
    return 0;
}

} // namespace pbr
