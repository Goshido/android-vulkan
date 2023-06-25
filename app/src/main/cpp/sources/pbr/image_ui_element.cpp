#include <pbr/image_ui_element.h>
#include <av_assert.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

ImageUIElement::ImageUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    std::string &&asset,
    CSSComputedValues &&css
) noexcept:
    UIElement ( true, parent ),
    _asset ( std::move ( asset ) ),
    _css ( css ),
    _isAutoWidth ( css._width.GetType () == LengthValue::eType::Auto ),
    _isAutoHeight ( css._height.GetType () == LengthValue::eType::Auto ),
    _isInlineBlock ( css._display == DisplayProperty::eValue::InlineBlock )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success )
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterImageUIElement" ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success )
    {
        android_vulkan::LogWarning ( "pbr::ImageUIElement::ImageUIElement - Can't append element inside Lua VM." );
        return;
    }

    auto const texture = UIPass::RequestImage ( _asset );

    if ( success = texture.has_value (); success )
    {
        _texture = *texture;
    }
}

ImageUIElement::~ImageUIElement () noexcept
{
    if ( _texture )
    {
        UIPass::ReleaseImage ( _texture );
    }
}

void ImageUIElement::ApplyLayout ( ApplyLayoutInfo &info ) noexcept
{
    if ( !_visible )
        return;

    // Method contains a lot of branchless optimizations.
    _parentLine = info._parentLine;
    _parentSize = info._canvasSize;

    GXVec2 const& canvasSize = info._canvasSize;
    CSSUnitToDevicePixel const& units = *info._cssUnits;

    _marginTopLeft = GXVec2 ( ResolvePixelLength ( _css._marginLeft, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._marginTop, canvasSize._data[ 1U ], true, units )
    );

    GXVec2 const paddingTopLeft ( ResolvePixelLength ( _css._paddingLeft, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._paddingTop, canvasSize._data[ 1U ], true, units )
    );

    GXVec2 marginBottomRight ( ResolvePixelLength ( _css._marginRight, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._marginBottom, canvasSize._data[ 1U ], true, units )
    );

    GXVec2 const paddingBottomRight ( ResolvePixelLength ( _css._paddingRight, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._paddingBottom, canvasSize._data[ 1U ], true, units )
    );

    _canvasTopLeftOffset.Sum ( _marginTopLeft, paddingTopLeft );

    GXVec2 penLocation {};
    float const& parentLeft = info._parentTopLeft._data[ 0U ];
    float const& parentWidth = canvasSize._data[ 0U ];
    size_t const oldVertices = info._vertices;

    auto const newLine = [ & ] () noexcept {
        GXVec2 beta {};
        beta.Sum ( _canvasTopLeftOffset, GXVec2 ( parentLeft, info._currentLineHeight ) );

        penLocation._data[ 0U ] = beta._data[ 0U ];
        penLocation._data[ 1U ] += beta._data[ 1U ];
        ++_parentLine;
    };

    switch ( _css._position )
    {
        case PositionProperty::eValue::Absolute:
            penLocation.Sum ( info._parentTopLeft, _canvasTopLeftOffset );
            _parentLine = 0U;
        break;

        case PositionProperty::eValue::Static:
        {
            _parentLine = info._parentLine;

            if ( parentLeft == info._penLocation._data[ 0U ] )
                break;

            if ( !_isInlineBlock )
            {
                newLine ();
                break;
            }

            penLocation.Sum ( info._penLocation, _canvasTopLeftOffset );
        }
        break;

        default:
            AV_ASSERT ( false )
        return;
    }

    _borderSize = ResolveSize ( canvasSize, units );
    GXVec2 beta {};
    beta.Sum ( _borderSize, _canvasTopLeftOffset );

    GXVec2 gamma {};
    gamma.Sum ( marginBottomRight, paddingBottomRight );

    _blockSize.Sum ( beta, gamma );

    if ( _css._position == PositionProperty::eValue::Absolute )
        return;

    // 'static' position territory

    if ( ( _blockSize._data[ 0U ] <= 0.0F ) | ( _blockSize._data[ 1U ] <= 0.0F ) )
        return;

    info._vertices += UIPass::GetVerticesPerRectangle ();

    if ( _css._display == DisplayProperty::eValue::Block )
    {
        // Block starts from new line and consumes whole parent block line.
        float const cases[] = { _blockSize._data[ 1U ], info._currentLineHeight };
        auto const s = static_cast<size_t> ( info._currentLineHeight != 0.0F );

        info._currentLineHeight = cases[ s ];
        info._parentLine += s;
        info._newLineHeight = _blockSize._data[ 1U ];
        info._penLocation._data[ 0U ] = parentLeft + _blockSize._data[ 0U ];
        info._penLocation._data[ 1U ] = info._parentTopLeft._data[ 1U ];
        return;
    }

    // 'inline-block' territory.

    bool const firstBlock = info._penLocation.IsEqual ( info._parentTopLeft );
    float const rest = parentWidth + parentLeft - info._penLocation._data[ 0U ];
    bool const blockCanFit = rest >= _blockSize._data[ 0U ];

    if ( firstBlock | blockCanFit )
    {
        info._currentLineHeight = std::max ( info._currentLineHeight, _blockSize._data[ 1U ] );
        info._penLocation._data[ 0U ] += _blockSize._data[ 0U ];
        return;
    }

    // Block goes to the new line of parent block.
    ++info._parentLine;
    info._penLocation._data[ 0U ] = info._parentTopLeft._data[ 0U ];
    info._penLocation._data[ 1U ] += info._currentLineHeight;
    info._newLineHeight = info._currentLineHeight;
    info._currentLineHeight = 0.0F;
    info._vertices = oldVertices;

    ApplyLayout ( info );
}

void ImageUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    constexpr GXColorRGB white ( 1.0F, 1.0F, 1.0F, 1.0F );
    constexpr GXVec2 imageTopLeft ( 0.0F, 0.0F );
    constexpr GXVec2 imageBottomRight ( 1.0F, 1.0F );

    AlignHander const handler = ResolveAlignment ( _parent );

    float const& penX = info._pen._data[ 0U ];
    float const& borderOffsetX = _canvasTopLeftOffset._data[ 0U ];
    float const& parentLeft = _parentSize._data[ 0U ];
    float const& blockWidth = _blockSize._data[ 0U ];

    GXVec2 topLeft {};
    topLeft._data[ 0U ] = handler ( penX, parentLeft, blockWidth ) + borderOffsetX;
    topLeft._data[ 1U ] = info._parentTopLeft._data[ 1U ] + info._parentLineOffsets[ _parentLine ];

    GXVec2 bottomRight {};
    bottomRight.Sum ( topLeft, _borderSize );

    FontStorage::GlyphInfo const& g = info._fontStorage->GetTransparentGlyphInfo ();
    UIVertexBuffer& vertexBuffer = info._vertexBuffer;

    UIPass::AppendRectangle ( vertexBuffer.data (),
        white,
        topLeft,
        bottomRight,
        g._topLeft,
        g._bottomRight,
        imageTopLeft,
        imageBottomRight
    );

    vertexBuffer = vertexBuffer.subspan ( UIPass::GetVerticesPerRectangle () );
    info._uiPass->SubmitImage ( _texture );
}

GXVec2 ImageUIElement::ResolveSize ( GXVec2 const& parentCanvasSize, CSSUnitToDevicePixel const& units ) noexcept
{
    if ( !_isAutoWidth & _isAutoHeight )
        return ResolveSizeByWidth ( parentCanvasSize._data[ 0U ], units );

    if ( _isAutoWidth & !_isAutoHeight )
        return ResolveSizeByHeight ( parentCanvasSize._data[ 1U ], units );

    return GXVec2 ( ResolvePixelLength ( _css._width, parentCanvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._height, parentCanvasSize._data[ 1U ], true, units )
    );
}

GXVec2 ImageUIElement::ResolveSizeByWidth ( float parentWidth, CSSUnitToDevicePixel const &units ) noexcept
{
    VkExtent2D const &r = _texture->GetResolution ();

    GXVec2 result {};
    result._data[ 0U ] = ResolvePixelLength ( _css._width, parentWidth, false, units );
    result._data[ 1U ] = result._data[ 0U ] * static_cast<float> ( r.height ) / static_cast<float> ( r.width );

    return result;
}

GXVec2 ImageUIElement::ResolveSizeByHeight ( float parentHeight, CSSUnitToDevicePixel const &units ) noexcept
{
    VkExtent2D const &r = _texture->GetResolution ();

    GXVec2 result {};
    result._data[ 1U ] = ResolvePixelLength ( _css._height, parentHeight, true, units );
    result._data[ 0U ] = result._data[ 1U ] * static_cast<float> ( r.width ) / static_cast<float> ( r.height );

    return result;
}

} // namespace pbr
