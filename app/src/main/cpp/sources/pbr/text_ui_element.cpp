#include <pbr/div_ui_element.h>
#include <pbr/text_ui_element.h>
#include <pbr/utf8_parser.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

TextUIElement::TextUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    std::u32string &&text
) noexcept:
    UIElement ( true, parent ),
    _text ( std::move ( text ) )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success )
    {
        android_vulkan::LogError ( "pbr::TextUIElement::TextUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterTextUIElement" ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogError ( "pbr::TextUIElement::TextUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success )
    {
        android_vulkan::LogWarning ( "pbr::TextUIElement::TextUIElement - Can't append element inside Lua VM." );
    }
}

void TextUIElement::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
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

void TextUIElement::ApplyLayout ( android_vulkan::Renderer &/*renderer*/,
    GXVec2 &/*penLocation*/,
    float &/*lineHeight*/,
    GXVec2 const &/*canvasSize*/,
    float /*parentLeft*/,
    float /*parentWidth*/
) noexcept
{
    if ( !_visible )
        return;

    std::string const& font = *ResolveFont ();
    (void)font;
}

void TextUIElement::Render () noexcept
{
    // TODO
}

[[maybe_unused]] GXColorRGB const* TextUIElement::ResolveColor () const noexcept
{
    if ( _color )
        return &_color.value ();

    for ( UIElement const* p = _parent; p; p = p->_parent )
    {
        // NOLINTNEXTLINE - downcast.
        auto const& div = *static_cast<DIVUIElement const*> ( p );
        ColorValue const& color = div._css._color;

        if ( !color.IsInherit () )
        {
            return &color.GetValue ();
        }
    }

    assert ( false );
    return nullptr;
}

std::string const* TextUIElement::ResolveFont () const noexcept
{
    for ( UIElement const* p = _parent; p; p = p->_parent )
    {
        // NOLINTNEXTLINE - downcast.
        auto const& div = *static_cast<DIVUIElement const*> ( p );
        std::string const& fontFile = div._css._fontFile;

        if ( !fontFile.empty () )
        {
            return &fontFile;
        }
    }

    assert ( false );
    return nullptr;
}

int TextUIElement::OnSetColorHSV ( lua_State* state )
{
    auto const h = static_cast<float> ( lua_tonumber ( state, 2 ) );
    auto const s = static_cast<float> ( lua_tonumber ( state, 3 ) );
    auto const v = static_cast<float> ( lua_tonumber ( state, 4 ) );
    auto const a = static_cast<float> ( lua_tonumber ( state, 5 ) );

    auto& self = *static_cast<TextUIElement*> ( lua_touserdata ( state, 1 ) );
    self._color = GXColorRGB ( GXColorHSV ( h, s, v, a ) );
    return 0;
}

int TextUIElement::OnSetColorRGB ( lua_State* state )
{
    auto const r = static_cast<GXUByte> ( lua_tonumber ( state, 2 ) );
    auto const g = static_cast<GXUByte> ( lua_tonumber ( state, 3 ) );
    auto const b = static_cast<GXUByte> ( lua_tonumber ( state, 4 ) );
    auto const a = static_cast<GXUByte> ( lua_tonumber ( state, 5 ) );

    auto& self = *static_cast<TextUIElement*> ( lua_touserdata ( state, 1 ) );
    self._color = GXColorRGB ( r, g, b, a );
    return 0;
}

int TextUIElement::OnSetText ( lua_State* state )
{
    auto str = UTF8Parser::ToU32String ( lua_tostring ( state, 2 ) );

    if ( !str )
        return 0;

    auto& self = *static_cast<TextUIElement*> ( lua_touserdata ( state, 1 ) );
    self._text = std::move ( *str );
    return 0;
}

} // namespace pbr
