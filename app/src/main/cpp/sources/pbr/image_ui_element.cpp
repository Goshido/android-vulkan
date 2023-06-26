#include <pbr/div_ui_element.h>
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
    std::vector<float>& lineHeights = *info._lineHeights;
    _parentLine = lineHeights.size () - 1U;

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
        beta.Sum ( _canvasTopLeftOffset, GXVec2 ( parentLeft, lineHeights.back () ) );

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
            if ( parentLeft == info._penLocation._data[ 0U ] )
            {
                penLocation.Sum ( info._penLocation, _canvasTopLeftOffset );
                break;
            }

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
        float const currentLineHeight = lineHeights.back ();

        float const cases[] = { _blockSize._data[ 1U ], currentLineHeight };
        auto const s = static_cast<size_t> ( currentLineHeight != 0.0F );

        lineHeights.back () = cases[ s ];
        info._penLocation._data[ 0U ] = info._parentTopLeft._data[ 0U ];
        info._penLocation._data[ 1U ] = info._penLocation._data[ 1U ] + cases[ s ];
        return;
    }

    // 'inline-block' territory.

    bool const firstBlock = info._penLocation.IsEqual ( info._parentTopLeft );
    float const rest = parentWidth + parentLeft - info._penLocation._data[ 0U ];
    bool const blockCanFit = rest >= _blockSize._data[ 0U ];

    if ( firstBlock | blockCanFit )
    {
        lineHeights.back () = std::max ( lineHeights.back (), _blockSize._data[ 1U ] );
        info._penLocation._data[ 0U ] += _blockSize._data[ 0U ];
        return;
    }

    // Block goes to the new line of parent block.
    info._penLocation._data[ 0U ] = info._parentTopLeft._data[ 0U ];
    info._penLocation._data[ 1U ] += lineHeights.back ();
    info._lineHeights->push_back ( 0.0F );
    info._vertices = oldVertices;

    ApplyLayout ( info );
}

void ImageUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    GXVec2 pen = info._pen;

    if ( info._line != _parentLine )
    {
        pen._data[ 0U ] = info._parentTopLeft._data[ 0U ];
        pen._data[ 1U ] += info._parentLineHeights[ info._line ];
    }

    AlignHander const verticalAlign = ResolveVerticalAlignment ( this );
    pen._data[ 1U ] = verticalAlign ( pen._data[ 1U ], info._parentLineHeights[ _parentLine ], _blockSize._data[ 1U ] );

    GXVec2 topLeft {};
    topLeft.Sum ( pen, _canvasTopLeftOffset );

    float const& penX = info._pen._data[ 0U ];
    float const& borderOffsetX = _canvasTopLeftOffset._data[ 0U ];
    float const& parentLeft = _parentSize._data[ 0U ];
    float const& blockWidth = _blockSize._data[ 0U ];

    AlignHander const handler = ResolveTextAlignment ( _parent );
    topLeft._data[ 0U ] = handler ( penX, parentLeft, blockWidth ) + borderOffsetX;

    GXVec2 bottomRight {};
    bottomRight.Sum ( topLeft, _borderSize );

    constexpr GXColorRGB white ( 1.0F, 1.0F, 1.0F, 1.0F );
    constexpr GXVec2 imageTopLeft ( 0.0F, 0.0F );
    constexpr GXVec2 imageBottomRight ( 1.0F, 1.0F );

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
