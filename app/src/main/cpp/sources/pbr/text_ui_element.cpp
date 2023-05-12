#include <pbr/text_ui_element.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

TextUIElement::TextUIElement ( std::u32string &&text ) noexcept:
    UIElement ( true ),
    _text ( std::move ( text ) )
{
    // NOTHING
}

void TextUIElement::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_TextUIElementCollectGarbage",
            .func = &TextUIElement::OnGarbageCollected
        },
        {
            .name = "av_TextUIElementHide",
            .func = &TextUIElement::OnHide
        },
        {
            .name = "av_TextUIElementShow",
            .func = &TextUIElement::OnShow
        },
        {
            .name = "av_TextUIElementSetColorHSV",
            .func = &TextUIElement::OnSetColorHSV
        },
        {
            .name = "av_TextUIElementSetColorRGB",
            .func = &TextUIElement::OnSetColorRGB
        },
        {
            .name = "av_TextUIElementSetText",
            .func = &TextUIElement::OnSetText
        }
    };

    for ( auto const& extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void TextUIElement::ApplyLayout () noexcept
{
    // TODO
}

void TextUIElement::Render () noexcept
{
    // TODO
}

int TextUIElement::OnGarbageCollected ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int TextUIElement::OnHide ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int TextUIElement::OnSetColorHSV ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int TextUIElement::OnSetColorRGB ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int TextUIElement::OnSetText ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int TextUIElement::OnShow ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
