#include <pbr/ui_element.h>


namespace pbr {

UIElement::UIElement ( bool visible ) noexcept:
    _visible ( visible )
{
    // NOTHING
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
