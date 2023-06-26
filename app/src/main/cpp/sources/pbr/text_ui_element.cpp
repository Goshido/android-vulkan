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
    _glyphs.resize ( _text.size () );

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

    size_t const glyphCount = _text.size ();
    _lines.clear ();

    // Estimation from top.
    _lines.reserve ( glyphCount );

    _glyphs.clear ();
    _glyphs.reserve ( glyphCount );

    FontStorage& fontStorage = *info._fontStorage;

    std::string const& fontAsset = *ResolveFont ();
    auto const size = static_cast<uint32_t> ( ResolveFontSize ( *info._cssUnits, *_parent ) );
    auto font = fontStorage.GetFont ( fontAsset, size );

    if ( !font )
        return;

    auto f = *font;
    constexpr size_t firstLineHeightIdx = 1U;

    std::vector<float>& divLineHeights = *info._lineHeights;
    size_t const startLine = divLineHeights.size () - 1U;
    size_t parentLine = startLine;

    float& currentLineHeight = divLineHeights.back ();
    auto const currentLineHeightInteger = static_cast<int32_t> ( currentLineHeight );
    _fontSize = f->second._lineHeight;
    int32_t const lineHeights[] = { _fontSize, std::max ( _fontSize, currentLineHeightInteger ) };

    GXVec2& penLocation = info._penLocation;
    float const canvasWidth = info._canvasSize._data[ 0U ];
    float const parentLeft = info._parentTopLeft._data[ 0U ];

    auto const l = static_cast<int32_t> ( parentLeft );
    auto const w = static_cast<int32_t> ( parentLeft + canvasWidth );

    auto x = static_cast<int32_t> ( penLocation._data[ 0U ] );
    auto start = x;

    size_t glyphsPerLine = 0U;

    float dummy;
    float const fraction = std::modf ( penLocation._data[ 1U ], &dummy );
    auto y = static_cast<int32_t> ( penLocation._data[ 1U ] );

    std::u32string_view text = _text;

    char32_t leftCharacter = 0;
    android_vulkan::Renderer& renderer = *info._renderer;

    auto const appendGlyph = [ this ] ( size_t line, FontStorage::GlyphInfo const &glyphInfo ) noexcept {
        _glyphs.emplace_back (
            Glyph
            {
                ._advance = 0,
                ._atlasTopLeft = glyphInfo._topLeft,
                ._atlasBottomRight = glyphInfo._bottomRight,
                ._offsetY = glyphInfo._offsetY,
                ._parentLine = line,
                ._width = glyphInfo._width,
                ._height = glyphInfo._height
            }
        );
    };

    FontStorage::GlyphInfo const* gi;
    int32_t previousX;

    {
        // Unroll first iteration of traverse loop to reduce amount of branching.
        char32_t const rightCharacter = text[ 0U ];
        text = text.substr ( 1U );

        gi = &fontStorage.GetGlyphInfo ( renderer, f, rightCharacter );
        previousX = x;
        x += gi->_advance + FontStorage::GetKerning ( f, leftCharacter, rightCharacter );
        leftCharacter = rightCharacter;
        ++glyphsPerLine;

        if ( x < w )
        {
            appendGlyph ( parentLine, *gi );
        }
        else
        {
            _lines.emplace_back ( Line {} );
            start = l;

            ++parentLine;
            y += currentLineHeightInteger;
            x = l + gi->_advance;

            appendGlyph ( parentLine, *gi );
            divLineHeights.push_back ( static_cast<float> ( _fontSize ) );
        }
    }

    Glyph* glyph = _glyphs.data ();

    constexpr size_t firstLineChangedIdx = 1U;
    bool firstLineState[] = { false, false };

    for ( char32_t const rightCharacter : text )
    {
        gi = &fontStorage.GetGlyphInfo ( renderer, f, rightCharacter );
        int32_t const baseX = x;
        int32_t const advance = gi->_advance + FontStorage::GetKerning ( f, leftCharacter, rightCharacter );
        x += advance;
        leftCharacter = rightCharacter;
        ++glyphsPerLine;

        if ( x < w )
        {
            glyph->_advance = baseX - previousX;
            ++glyph;

            previousX = baseX;

            appendGlyph ( parentLine, *gi );
            firstLineState[ static_cast<size_t> ( parentLine == startLine ) ] = true;
            continue;
        }

        _lines.emplace_back (
            Line
            {
                ._glyphs = glyphsPerLine - 1U,
                ._length = x - ( advance + start )
            }
        );

        start = l;
        glyphsPerLine = 1U;
        ++glyph;

        x = l + gi->_advance;
        y += lineHeights[ static_cast<size_t> ( parentLine == startLine ) ];
        ++parentLine;

        appendGlyph ( parentLine, *gi );
        previousX = l;

        divLineHeights.push_back ( static_cast<float> ( _fontSize ) );
    }

    glyph->_advance = gi->_advance;

    _lines.emplace_back (
        Line
        {
            ._glyphs = glyphsPerLine,
            ._length = x + gi->_width - ( gi->_advance + start )
        }
    );

    int32_t const cases[] = { currentLineHeightInteger, lineHeights[ firstLineHeightIdx ] };
    currentLineHeight = static_cast<float> ( cases[ static_cast<size_t> ( firstLineState[ firstLineChangedIdx ] ) ] );

    penLocation._data[ 0U ] = static_cast<float> ( x );
    penLocation._data[ 1U ] = static_cast<float> ( y ) + fraction;

    info._vertices += _text.size () * UIPass::GetVerticesPerRectangle ();
}

void TextUIElement::Submit ( SubmitInfo &info ) noexcept
{
    size_t const glyphCount = _glyphs.size ();

    if ( !_visible | ( glyphCount == 0U ) )
        return;

    UIVertexBuffer& vertexBuffer = info._vertexBuffer;
    UIVertexInfo* v = vertexBuffer.data ();

    Glyph const* glyphs = _glyphs.data ();
    GXColorRGB const& color = ResolveColor ();

    constexpr size_t verticesPerGlyph = UIPass::GetVerticesPerRectangle ();
    constexpr GXVec2 imageUV ( 0.5F, 0.5F );

    size_t limit = 0U;
    size_t i = 0U;
    GXVec2 pen = info._pen;

    size_t const firstLineIdx = _glyphs.front ()._parentLine;
    float const* height = info._parentLineHeights + firstLineIdx;

    if ( info._line != firstLineIdx )
    {
        pen._data[ 0U ] = info._parentTopLeft._data[ 0U ];
        pen._data[ 1U ] += *height;
        ++height;
    }

    AlignIntegerHander const textAlignment = ResolveIntegerTextAlignment ( _parent );
    AlignIntegerHander const verticalAlignment = ResolveIntegerVerticalAlignment ( _parent );

    auto x = static_cast<int32_t> ( pen._data[ 0U ] );
    auto y = static_cast<int32_t> ( pen._data[ 1U ] );

    auto const parentLeft = static_cast<int32_t> ( info._parentTopLeft._data[ 0U ] );
    auto const parentWidth =  static_cast<int32_t> ( info._parentWidth );

    for ( Line const& line : _lines )
    {
        x = textAlignment ( x, parentWidth, line._length );
        y = verticalAlignment ( y, static_cast<int32_t> ( *height ), _fontSize );
        limit += line._glyphs;

        for ( ; i < limit; ++i )
        {
            Glyph const& g = glyphs[ i ];

            int32_t const glyphTop = y + g._offsetY;
            int32_t const glyphBottom = glyphTop + g._height;
            int32_t const glyphRight = x + g._width;

            UIPass::AppendRectangle ( v,
                color,
                GXVec2 ( static_cast<float> ( x ), static_cast<float> ( glyphTop ) ),
                GXVec2 ( static_cast<float> ( glyphRight ), static_cast<float> ( glyphBottom ) ),
                g._atlasTopLeft,
                g._atlasBottomRight,
                imageUV,
                imageUV
            );

            x += g._advance;
            v += verticesPerGlyph;
        }

        pen._data[ 1U ] += *height;
        ++height;

        x = parentLeft;
        y = static_cast<int32_t> ( pen._data[ 1U ] );
    }

    size_t const vertexCount = glyphCount * verticesPerGlyph;
    vertexBuffer = vertexBuffer.subspan ( vertexCount );
    info._pen = GXVec2 ( static_cast<float> ( x ), pen._data[ 1U ] );
    info._uiPass->SubmitText ( vertexCount );
}

GXColorRGB const& TextUIElement::ResolveColor () const noexcept
{
    if ( _color )
        return _color.value ();

    for ( UIElement const* p = _parent; p; p = p->_parent )
    {
        // NOLINTNEXTLINE - downcast.
        auto const& div = *static_cast<DIVUIElement const*> ( p );
        ColorValue const& color = div._css._color;

        if ( !color.IsInherit () )
        {
            return color.GetValue ();
        }
    }

    android_vulkan::LogError ( "pbr::TextUIElement::ResolveColor - No color was found!" );
    AV_ASSERT ( false )

    constexpr static GXColorRGB nullColor ( 0.0F, 0.0F, 0.0F, 1.0F );
    return nullColor;
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

int32_t TextUIElement::AlignIntegerToCenter ( int32_t pen, int32_t parentSize, int32_t lineSize ) noexcept
{
    return pen + ( ( parentSize - lineSize ) / 2 );
}

int32_t TextUIElement::AlignIntegerToStart ( int32_t pen, int32_t /*parentSize*/, int32_t /*lineLength*/ ) noexcept
{
    return pen;
}

int32_t TextUIElement::AlignIntegerToEnd ( int32_t pen, int32_t parentSize, int32_t lineSize ) noexcept
{
    return pen + parentSize - lineSize;
}

TextUIElement::AlignIntegerHander TextUIElement::ResolveIntegerTextAlignment ( UIElement const* parent ) noexcept
{
    while ( parent )
    {
        // NOLINTNEXTLINE - downcast
        auto const& div = static_cast<DIVUIElement const&> ( *parent );

        switch ( div._css._textAlign )
        {
            case TextAlignProperty::eValue::Center:
            return &TextUIElement::AlignIntegerToCenter;

            case TextAlignProperty::eValue::Left:
            return &TextUIElement::AlignIntegerToStart;

            case TextAlignProperty::eValue::Right:
            return &TextUIElement::AlignIntegerToEnd;

            case TextAlignProperty::eValue::Inherit:
                parent = parent->_parent;
            break;
        }
    }

    AV_ASSERT ( false )
    return &TextUIElement::AlignIntegerToStart;
}

TextUIElement::AlignIntegerHander TextUIElement::ResolveIntegerVerticalAlignment ( UIElement const* parent ) noexcept
{
    while ( parent )
    {
        // NOLINTNEXTLINE - downcast
        auto const& div = static_cast<DIVUIElement const&> ( *parent );

        switch ( div._css._verticalAlign )
        {
            case VerticalAlignProperty::eValue::Bottom:
            return &TextUIElement::AlignIntegerToEnd;

            case VerticalAlignProperty::eValue::Middle:
            return &TextUIElement::AlignIntegerToCenter;

            case VerticalAlignProperty::eValue::Top:
            return &TextUIElement::AlignIntegerToStart;

            case VerticalAlignProperty::eValue::Inherit:
                parent = parent->_parent;
            break;
        }
    }

    AV_ASSERT ( false )
    return &TextUIElement::AlignIntegerToStart;
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
    self._glyphs.resize ( self._text.size () );
    return 0;
}

} // namespace pbr
