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

    _glyphs.clear ();
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

    auto const appendGlyph = [ this ] ( int32_t x, int32_t y, FontStorage::GlyphInfo const &glyphInfo ) noexcept {
        y += glyphInfo._offsetY;

        _glyphs.emplace_back (
            Glyph
            {
                ._topLeft = GXVec2 ( static_cast<float> ( x ), static_cast<float> ( y ) ),

                ._bottomRight = GXVec2 ( static_cast<float> ( x + glyphInfo._width ),
                    static_cast<float> ( y + glyphInfo._height )
                ),

                ._atlasTopLeft = glyphInfo._topLeft,
                ._atlasBottomRight = glyphInfo._bottomRight
            }
        );
    };

    {
        // Unroll first iteration of traverse loop to reduce amount of branching.
        char32_t const rightCharacter = text[ 0U ];
        text = text.substr ( 1U );

        FontStorage::GlyphInfo const& gi = fontStorage.GetGlyphInfo ( renderer, f, rightCharacter );
        int32_t const baseX = x;
        x += gi._advance + FontStorage::GetKerning ( f, leftCharacter, rightCharacter );
        leftCharacter = rightCharacter;

        if ( x < w )
        {
            appendGlyph ( baseX, y, gi );
        }
        else
        {
            y += currentLineHeightInteger;
            appendGlyph ( baseX, y, gi );
            x = l + gi._advance;
            line = 1U;
        }
    }

    constexpr size_t firstLineChangedIdx = 1U;
    bool firstLineState[] = { false, false };

    for ( char32_t const rightCharacter : text )
    {
        FontStorage::GlyphInfo const& gi = fontStorage.GetGlyphInfo ( renderer, f, rightCharacter );
        int32_t const baseX = x;
        x += gi._advance + FontStorage::GetKerning ( f, leftCharacter, rightCharacter );
        leftCharacter = rightCharacter;

        if ( x < w )
        {
            appendGlyph ( baseX, y, gi );
            firstLineState[ static_cast<size_t> ( line == 0U ) ] = true;
            continue;
        }

        x = l + gi._advance;
        y += lineHeights[ static_cast<size_t> ( line == 0U ) ];
        appendGlyph ( baseX, y, gi );
        ++line;
    }

    int32_t const cases[] = { currentLineHeightInteger, lineHeights[ firstLineHeightIdx ] };
    currentLineHeight = static_cast<float> ( cases[ static_cast<size_t> ( firstLineState[ firstLineChangedIdx ] ) ] );

    info._newLineHeight = static_cast<float> ( lineHeights[ fontHeightIdx ] );
    info._newLines = line;

    penLocation._data[ 0U ] = static_cast<float> ( x );
    penLocation._data[ 1U ] = static_cast<float> ( y ) + fraction;

    // One glyph is one rectangle (two triangles).
    info._vertices += 6U * text.size ();
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

    constexpr size_t verticesPerGlyph = 6U;
    constexpr GXVec2 imageUV ( 0.5F, 0.5F );

    for ( size_t i = 0U; i < glyphCount; ++i )
    {
        Glyph const& g = glyphs[ i ];

        v[ 0U ] =
        {
            ._vertex = g._topLeft,
            ._color = color,
            ._atlas = g._atlasTopLeft,
            ._imageUV = imageUV
        };

        v[ 1U ] =
        {
            ._vertex = GXVec2 ( g._bottomRight._data[ 0U ], g._topLeft._data[ 1U ] ),
            ._color = color,

            ._atlas =
                GXVec3 ( g._atlasBottomRight._data[ 0U ], g._atlasTopLeft._data[ 1U ], g._atlasTopLeft._data[ 2U ] ),

            ._imageUV = imageUV
        };

        v[ 2U ] =
        {
            ._vertex = g._bottomRight,
            ._color = color,
            ._atlas = g._atlasBottomRight,
            ._imageUV = imageUV
        };

        v[ 3U ] =
        {
            ._vertex = g._bottomRight,
            ._color = color,
            ._atlas = g._atlasBottomRight,
            ._imageUV = imageUV
        };

        v[ 4U ] =
        {
            ._vertex = GXVec2 ( g._topLeft._data[ 0U ], g._bottomRight._data[ 1U ] ),
            ._color = color,

            ._atlas =
                GXVec3 ( g._atlasTopLeft._data[ 0U ], g._atlasBottomRight._data[ 1U ], g._atlasTopLeft._data[ 2U ] ),

            ._imageUV = imageUV
        };

        v[ 5U ] =
        {
            ._vertex = g._topLeft,
            ._color = color,
            ._atlas = g._atlasTopLeft,
            ._imageUV = imageUV
        };

        v += verticesPerGlyph;
    }

    size_t const vertexCount = glyphCount * verticesPerGlyph;
    vertexBuffer = vertexBuffer.subspan ( vertexCount );
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
