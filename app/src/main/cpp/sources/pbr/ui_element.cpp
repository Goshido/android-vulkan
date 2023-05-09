#include <pbr/ui_element.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

UIElement::UIElement ( bool visible ) noexcept:
    _visible ( visible )
{
    // NOTHING
}

void UIElement::AppendChildElement ( std::unique_ptr<UIElement> &&/*element*/ ) noexcept
{
    // Something wrong with child element. System tried to add child element to something which does not support
    // child elements. For example: trying to add child element to text element.
    assert ( false );
}

int UIElement::OnShow ( lua_State* /*state*/ )
{
    return 0;
}

int UIElement::OnHide ( lua_State* /*state*/ )
{
    return 0;
}

} // namespace pbr
