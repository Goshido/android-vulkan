#include <precompiled_headers.hpp>
#include <pbr/div_ui_element.hpp>
#include <pbr/text_ui_element.hpp>
#include <pbr/utf8_parser.hpp>
#include <av_assert.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

bool TextUIElement::ApplyLayoutCache::Run ( ApplyInfo &info ) noexcept
{
    if ( _lineHeights.empty () )
        return false;

    bool const c0 = _isTextChanged;
    bool const c1 = _lineHeights.front () < info._lineHeights->back ();
    bool const c2 = !_penIn.IsEqual ( info._pen );

    _isTextChanged = false;

    if ( c0 | c1 | c2 )
        return false;

    info._pen = _penOut;
    info._vertices += _vertices;

    std::vector<float> &lines = *info._lineHeights;
    lines.pop_back ();
    lines.insert ( lines.cend (), _lineHeights.cbegin (), _lineHeights.cend () );

    return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool TextUIElement::SubmitCache::Run ( UpdateInfo &info,
    TextAlignProperty::eValue horizontal,
    VerticalAlignProperty::eValue vertical,
    std::vector<float> const &cachedLineHeight
) noexcept
{
    bool const c0 = _isTextChanged;
    bool const c1 = _isColorChanged;

    std::span<float const> const dst ( info._parentLineHeights + info._line, cachedLineHeight.size () );
    bool const c2 = !std::equal ( dst.begin (), dst.end (), cachedLineHeight.cbegin () );

    bool c3 = false;

    switch ( horizontal )
    {
        case TextAlignProperty::eValue::Right:
            [[fallthrough]];
        case TextAlignProperty::eValue::Center:
            c3 = !_parenSize.IsEqual ( info._parentSize );
        break;

        case TextAlignProperty::eValue::Left:
            c3 = !_penIn.IsEqual ( info._pen );
        break;

        case TextAlignProperty::eValue::Inherit:
            AV_ASSERT ( false )
        break;
    }

    bool c4 = false;

    switch ( vertical )
    {
        case VerticalAlignProperty::eValue::Bottom:
            [[fallthrough]];
        case VerticalAlignProperty::eValue::Middle:
            c4 = !_parenSize.IsEqual ( info._parentSize );
        break;

        case VerticalAlignProperty::eValue::Top:
            c4 = !_penIn.IsEqual ( info._pen );
        break;

        case VerticalAlignProperty::eValue::Inherit:
            AV_ASSERT ( false )
        break;
    }

    _isTextChanged = false;
    _isColorChanged = false;

    if ( c0 | c1 | c2 | c3 | c4 ) [[likely]]
        return false;

    info._pen = _penOut;
    return true;
}

//----------------------------------------------------------------------------------------------------------------------

TextUIElement::TextUIElement ( bool visible, UIElement const* parent, std::u32string &&text ) noexcept:
    UIElement ( visible, parent ),
    _text ( std::move ( text ) )
{
    _glyphs.resize ( _text.size () );
}

TextUIElement::TextUIElement ( bool visible,
    UIElement const* parent,
    std::u32string &&text,
    std::string &&name
) noexcept:
    UIElement ( visible, parent, std::move ( name ) ),
    _text ( std::move ( text ) )
{
    _glyphs.resize ( _text.size () );
}

TextUIElement::TextUIElement ( bool visible, UIElement const* parent, std::string_view text ) noexcept:
    UIElement ( visible, parent )
{
    if ( auto str = UTF8Parser::ToU32String ( text ); str ) [[likely]]
        _text = std::move ( *str );

    _glyphs.resize ( _text.size () );
}

TextUIElement::TextUIElement ( bool visible,
    UIElement const* parent,
    std::string_view text,
    std::string &&name
) noexcept:
    UIElement ( visible, parent, std::move ( name ) )
{
    if ( auto str = UTF8Parser::ToU32String ( text ); str ) [[likely]]
        _text = std::move ( *str );

    _glyphs.resize ( _text.size () );
}

void TextUIElement::SetColor ( GXColorUNORM color ) noexcept
{
    _color = color;
    _submitCache._isColorChanged = true;
}

void TextUIElement::SetText ( char const* text ) noexcept
{
    SetText ( std::string_view ( text ) );
}

void TextUIElement::SetText ( std::string_view text ) noexcept
{
    auto str = UTF8Parser::ToU32String ( text );

    if ( !str || str == _text )
        return;

    _text = std::move ( *str );
    _glyphs.resize ( _text.size () );
    _applyLayoutCache._isTextChanged = true;
    _submitCache._isTextChanged = true;
}

void TextUIElement::ApplyLayout ( ApplyInfo &info ) noexcept
{
    bool const isEmpty = _text.empty ();

    if ( ( !_visible | isEmpty ) || _applyLayoutCache.Run ( info ) )
        return;

    info._hasChanges = true;
    size_t const glyphCount = _text.size ();
    _lines.clear ();
    _applyLayoutCache._lineHeights.clear ();

    // Estimation from top.
    _lines.reserve ( glyphCount );
    _applyLayoutCache._lineHeights.reserve ( glyphCount );

    _glyphs.clear ();
    _glyphs.reserve ( glyphCount );

    FontStorage &fontStorage = *info._fontStorage;

    std::string const &fontAsset = *ResolveFont ();
    auto const size = static_cast<uint32_t> ( ResolveFontSize ( *_parent ) );
    auto font = fontStorage.GetFont ( fontAsset, size );

    if ( !font )
        return;

    auto f = *font;
    constexpr size_t firstLineHeightIdx = 1U;

    std::vector<float> &divLineHeights = *info._lineHeights;
    size_t const startLine = divLineHeights.size () - 1U;
    size_t parentLine = startLine;

    float const currentLineHeight = divLineHeights.back ();
    _applyLayoutCache._lineHeights.push_back ( currentLineHeight );

    auto const currentLineHeightInteger = static_cast<int32_t> ( currentLineHeight );

    _fontSize = f->second._lineHeight;
    auto const fontSizeF = static_cast<float> ( _fontSize );

    int32_t const lineHeights[] = { _fontSize, std::max ( _fontSize, currentLineHeightInteger ) };
    float const lineHeightsF[] = { fontSizeF, std::max ( fontSizeF, currentLineHeight ) };

    GXVec2 &pen = info._pen;
    _applyLayoutCache._penIn = pen;

    float const canvasWidth = info._canvasSize._data[ 0U ];
    auto const w = static_cast<int32_t> ( canvasWidth );

    auto x = static_cast<int32_t> ( pen._data[ 0U ] );
    auto start = x;

    size_t glyphsPerLine = 0U;

    float dummy;
    float const fraction = std::modf ( pen._data[ 1U ], &dummy );
    auto y = static_cast<int32_t> ( pen._data[ 1U ] );

    std::u32string_view text = _text;

    char32_t leftCharacter = 0;
    android_vulkan::Renderer &renderer = *info._renderer;

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

            float const h = lineHeightsF[ firstLineHeightIdx ];
            divLineHeights.back () = h;
            _applyLayoutCache._lineHeights.back () = h;
        }
        else
        {
            _lines.emplace_back ( Line {} );
            start = 0;

            ++parentLine;
            y += currentLineHeightInteger;
            x = gi->_advance;

            appendGlyph ( parentLine, *gi );
            divLineHeights.push_back ( fontSizeF );
            _applyLayoutCache._lineHeights.push_back ( fontSizeF );
        }
    }

    Glyph* glyph = _glyphs.data ();

    for ( char32_t const rightCharacter : text )
    {
        gi = &fontStorage.GetGlyphInfo ( renderer, f, rightCharacter );
        int32_t const baseX = x;
        int32_t const advance = gi->_advance + FontStorage::GetKerning ( f, leftCharacter, rightCharacter );
        x += advance;
        leftCharacter = rightCharacter;

        if ( x < w )
        {
            ++glyphsPerLine;

            glyph->_advance = baseX - previousX;
            ++glyph;

            previousX = baseX;
            appendGlyph ( parentLine, *gi );

            float const h = lineHeightsF[ static_cast<size_t> ( parentLine == startLine ) ];
            divLineHeights.back () = h;
            _applyLayoutCache._lineHeights.back () = h;
            continue;
        }

        _lines.emplace_back (
            Line
            {
                ._glyphs = glyphsPerLine - 1U,
                ._length = x - ( advance + start )
            }
        );

        start = 0;
        glyphsPerLine = 1U;
        ++glyph;

        x = gi->_advance;
        y += lineHeights[ static_cast<size_t> ( parentLine == startLine ) ];
        ++parentLine;

        appendGlyph ( parentLine, *gi );
        previousX = 0;

        divLineHeights.push_back ( fontSizeF );
        _applyLayoutCache._lineHeights.push_back ( fontSizeF );
    }

    glyph->_advance = gi->_advance;

    _lines.emplace_back (
        Line
        {
            ._glyphs = glyphsPerLine,
            ._length = x - start
        }
    );

    pen._data[ 0U ] = static_cast<float> ( x );
    pen._data[ 1U ] = static_cast<float> ( y ) + fraction;
    _applyLayoutCache._penOut = pen;

    size_t const vertices = _text.size () * UIPass::GetVerticesPerRectangle ();
    _applyLayoutCache._vertices = vertices;
    info._vertices += vertices;
}

void TextUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    size_t const vertices = _submitCache._positions.size ();
    UIVertexBuffer &uiVertexBuffer = info._vertexBuffer;
    std::span<GXVec2> &uiPositions = uiVertexBuffer._positions;
    std::span<UIVertex> &uiVertices = uiVertexBuffer._vertices;

    std::memcpy ( uiPositions.data (), _submitCache._positions.data (), _submitCache._positionBufferBytes );
    std::memcpy ( uiVertices.data (), _submitCache._vertices.data (), _submitCache._vertexBufferBytes );

    uiPositions = uiPositions.subspan ( vertices );
    uiVertices = uiVertices.subspan ( vertices );

    info._uiPass->SubmitText ( vertices );
}

bool TextUIElement::UpdateCache ( UpdateInfo &info ) noexcept
{
    size_t const glyphCount = _glyphs.size ();
    bool const needRefill = _visibilityChanged;
    _visibilityChanged = false;

    if ( !_visible | ( glyphCount == 0U ) )
        return needRefill;

    if ( _submitCache.Run ( info, GetTextAlignment (), GetVerticalAlignment (), _applyLayoutCache._lineHeights ) )
        return needRefill;

    _submitCache._parenSize = info._parentSize;
    _submitCache._penIn = info._pen;

    std::vector<GXVec2> &positionBuffer = _submitCache._positions;
    positionBuffer.clear ();

    std::vector<UIVertex> &vertexBuffer = _submitCache._vertices;
    vertexBuffer.clear ();

    constexpr size_t verticesPerGlyph = UIPass::GetVerticesPerRectangle ();
    size_t const vertexCount = glyphCount * verticesPerGlyph;
    positionBuffer.resize ( vertexCount );
    vertexBuffer.resize ( vertexCount );

    GXVec2* p = positionBuffer.data ();
    UIVertex* v = vertexBuffer.data ();

    Glyph const* glyphs = _glyphs.data ();
    GXColorUNORM const color = ResolveColor ();
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

    AlignIntegerHandler const textAlignment = GetIntegerTextAlignment ();
    AlignIntegerHandler const verticalAlignment = GetIntegerVerticalAlignment ();

    auto x = static_cast<int32_t> ( pen._data[ 0U ] );
    auto y = static_cast<int32_t> ( pen._data[ 1U ] );

    auto const parentLeft = static_cast<int32_t> ( info._parentTopLeft._data[ 0U ] );
    auto const parentWidth =  static_cast<int32_t> ( info._parentSize._data[ 0U ] );

    for ( Line const &line : _lines )
    {
        x = textAlignment ( x, parentWidth, line._length );
        y = verticalAlignment ( y, static_cast<int32_t> ( *height ), _fontSize );
        limit += line._glyphs;

        for ( ; i < limit; ++i )
        {
            Glyph const &g = glyphs[ i ];

            int32_t const glyphTop = y + g._offsetY;
            int32_t const glyphBottom = glyphTop + g._height;
            int32_t const glyphRight = x + g._width;

            UIPass::AppendRectangle ( p,
                v,
                color,
                GXVec2 ( static_cast<float> ( x ), static_cast<float> ( glyphTop ) ),
                GXVec2 ( static_cast<float> ( glyphRight ), static_cast<float> ( glyphBottom ) ),
                g._atlasTopLeft,
                g._atlasBottomRight,
                imageUV,
                imageUV
            );

            x += g._advance;

            p += verticesPerGlyph;
            v += verticesPerGlyph;
        }

        pen._data[ 1U ] += *height;
        ++height;

        x = parentLeft;
        y = static_cast<int32_t> ( pen._data[ 1U ] );
    }

    GXVec2 const penOut = GXVec2 ( static_cast<float> ( x ), pen._data[ 1U ] );
    _submitCache._penOut = penOut;
    info._pen = penOut;

    _submitCache._positionBufferBytes = vertexCount * sizeof ( GXVec2 );
    _submitCache._vertexBufferBytes = vertexCount * sizeof ( UIVertex );
    return true;
}

TextAlignProperty::eValue TextUIElement::GetTextAlignment () const noexcept
{
    for ( UIElement const* p = _parent; p; )
    {
        switch ( TextAlignProperty::eValue const align = p->GetCSS ()._textAlign; align )
        {
            case TextAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;

            case TextAlignProperty::eValue::Center:
                [[fallthrough]];
            case TextAlignProperty::eValue::Left:
                [[fallthrough]];
            case TextAlignProperty::eValue::Right:
            return align;
        }
    }

    AV_ASSERT ( false )
    return TextAlignProperty::eValue::Left;
}

VerticalAlignProperty::eValue TextUIElement::GetVerticalAlignment () const noexcept
{
    for ( UIElement const* p = _parent; p; )
    {
        switch ( VerticalAlignProperty::eValue const align = p->GetCSS ()._verticalAlign; align )
        {
            case VerticalAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;

            case VerticalAlignProperty::eValue::Bottom:
                [[fallthrough]];
            case VerticalAlignProperty::eValue::Middle:
                [[fallthrough]];
            case VerticalAlignProperty::eValue::Top:
            return align;
        }
    }

    AV_ASSERT ( false )
    return VerticalAlignProperty::eValue::Top;
}

TextUIElement::AlignIntegerHandler TextUIElement::GetIntegerTextAlignment () const noexcept
{
    for ( UIElement const* p = _parent; p; )
    {
        switch ( p->GetCSS ()._textAlign )
        {
            case TextAlignProperty::eValue::Center:
            return &TextUIElement::AlignIntegerToCenter;

            case TextAlignProperty::eValue::Left:
            return &TextUIElement::AlignIntegerToStart;

            case TextAlignProperty::eValue::Right:
            return &TextUIElement::AlignIntegerToEnd;

            case TextAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;
        }
    }

    AV_ASSERT ( false )
    return &TextUIElement::AlignIntegerToStart;
}

TextUIElement::AlignIntegerHandler TextUIElement::GetIntegerVerticalAlignment () const noexcept
{
    for ( UIElement const* p = _parent; p; )
    {
        switch ( p->GetCSS ()._verticalAlign )
        {
            case VerticalAlignProperty::eValue::Bottom:
            return &TextUIElement::AlignIntegerToEnd;

            case VerticalAlignProperty::eValue::Middle:
            return &TextUIElement::AlignIntegerToCenter;

            case VerticalAlignProperty::eValue::Top:
            return &TextUIElement::AlignIntegerToStart;

            case VerticalAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;
        }
    }

    AV_ASSERT ( false )
    return &TextUIElement::AlignIntegerToStart;
}

GXColorUNORM TextUIElement::ResolveColor () const noexcept
{
    if ( _color )
        return _color.value ();

    for ( UIElement const* p = _parent; p; p = p->_parent )
    {
        // NOLINTNEXTLINE - downcast.
        auto const &div = *static_cast<DIVUIElement const*> ( p );
        ColorValue const &color = div.GetCSS ()._color;

        if ( !color.IsInherit () )
        {
            return color.GetSRGB ();
        }
    }

    android_vulkan::LogError ( "pbr::TextUIElement::ResolveColor - No color was found!" );
    AV_ASSERT ( false )

    constexpr GXColorUNORM nullColor ( 0U, 0U, 0U, 0xFFU );
    return nullColor;
}

std::string const* TextUIElement::ResolveFont () const noexcept
{
    for ( UIElement const* p = _parent; p; p = p->_parent )
    {
        if ( std::string const &fontFile = p->GetCSS ()._fontFile; !fontFile.empty () )
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

} // namespace pbr
