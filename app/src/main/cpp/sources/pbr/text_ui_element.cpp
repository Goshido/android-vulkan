#include <pbr/text_ui_element.h>


namespace pbr {

TextUIElement::TextUIElement ( std::u32string &&text ) noexcept:
    UIElement ( true ),
    _text ( std::move ( text ) )
{
    // NOTHING
}

void TextUIElement::ApplyLayout () noexcept
{
    // TODO
}

void TextUIElement::Render () noexcept
{
    // TODO
}

int TextUIElement::OnSetColor ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int TextUIElement::OnSetText ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
