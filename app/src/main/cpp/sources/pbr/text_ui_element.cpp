#include <pbr/div_ui_element.h>
#include <pbr/text_ui_element.h>
#include <pbr/utf8_parser.h>
#include <av_assert.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

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

void TextUIElement::ApplyLayout ( ApplyLayoutInfo &info ) noexcept
{
    bool const isEmpty = _text.empty ();

    if ( !_visible | isEmpty )
        return;

    FontStorage& fontStorage = *info._fontStorage;

    std::string const& fontAsset = *ResolveFont ();
    auto const size = static_cast<uint32_t> ( ResolveFontSize ( *info._cssUnits, *_parent ) );
    auto font = fontStorage.GetFont ( fontAsset, size );

    if ( !font )
        return;

    auto f = *font;

    constexpr size_t fontHeightIdx = 0U;
    constexpr size_t firstLineHeightIdx = 1U;

    float& currentLineHeight = info._currentLineHeight;
    auto const currentLineHeightInteger = static_cast<int32_t> ( currentLineHeight );

    int32_t const lineHeights[] =
    {
        f->second._lineHeight,
        std::max ( f->second._lineHeight, currentLineHeightInteger )
    };

    GXVec2& penLocation = info._penLocation;

    auto const l = static_cast<int32_t> ( info._parentTopLeft._data[ 0U ] );
    auto const w = static_cast<int32_t> ( info._parentTopLeft._data[ 0U ] + info._canvasSize._data[ 0U ] );

    auto x = static_cast<int32_t> ( penLocation._data[ 0U ] );

    float dummy;
    float const fraction = std::modf ( penLocation._data[ 1U ], &dummy );
    auto y = static_cast<int32_t> ( penLocation._data[ 1U ] );

    std::u32string_view text = _text;

    size_t line = 0U;
    char32_t leftCharacter = 0;
    android_vulkan::Renderer& renderer = *info._renderer;

    {
        // Unroll first iteration of traverse loop to reduce amount of branching.
        char32_t const rightCharacter = text[ 0U ];
        text = text.substr ( 1U );

        FontStorage::GlyphInfo const& gi = fontStorage.GetGlyphInfo ( renderer, f, rightCharacter );
        x += gi._advance + fontStorage.GetKerning ( f, leftCharacter, rightCharacter );
        leftCharacter = rightCharacter;

        if ( x < w )
        {
            y += lineHeights[ firstLineHeightIdx ];
        }
        else
        {
            x = l + gi._advance;
            y += currentLineHeightInteger + lineHeights[ fontHeightIdx ];
            line = 1U;
        }
    }

    constexpr size_t firstLineChangedIdx = 1U;
    bool firstLineState[] = { false, false };

    for ( char32_t const rightCharacter : text )
    {
        FontStorage::GlyphInfo const& gi = fontStorage.GetGlyphInfo ( renderer, f, rightCharacter );
        x += gi._advance + fontStorage.GetKerning ( f, leftCharacter, rightCharacter );
        leftCharacter = rightCharacter;

        if ( x < w )
        {
            firstLineState[ static_cast<size_t> ( line == 0U ) ] = true;
            continue;
        }

        x = l + gi._advance;
        y += lineHeights[ static_cast<size_t> ( line == 0U ) ];
        ++line;
    }

    int32_t const cases[] = { currentLineHeightInteger, lineHeights[ firstLineHeightIdx ] };
    currentLineHeight = static_cast<float> ( cases[ static_cast<size_t> ( firstLineState[ firstLineChangedIdx ] ) ] );

    info._newLineHeight = static_cast<float> ( lineHeights[ fontHeightIdx ] );
    info._newLines = line;

    penLocation._data[ 0U ] = static_cast<float> ( x );
    penLocation._data[ 1U ] = fraction + static_cast<float> ( y - lineHeights[ static_cast<size_t> ( line == 0U ) ] );

    // One glyph is one rectangle (two triangles).
    info._vertices += 6U * text.size ();
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

    AV_ASSERT ( false )
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

    AV_ASSERT ( false )
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
