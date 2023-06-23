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

    GXVec2 const& canvasSize = info._canvasSize;
    CSSUnitToDevicePixel const& units = *info._cssUnits;

    GXVec2 marginTopLeft ( ResolvePixelLength ( _css._marginLeft, canvasSize._data[ 0U ], false, units ),
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

    GXVec2 alpha {};
    alpha.Sum ( marginTopLeft, paddingTopLeft );

    GXVec2 penLocation {};
    float const& parentLeft = info._parentTopLeft._data[ 0U ];
    float const& parentWidth = canvasSize._data[ 0U ];
    float const& paddingRight = paddingBottomRight._data[ 0U ];
    float const& marginRight = marginBottomRight._data[ 0U ];

    auto const newLine = [ & ] () noexcept {
        GXVec2 beta {};
        beta.Sum ( alpha, GXVec2 ( parentLeft, info._currentLineHeight ) );

        penLocation._data[ 0U ] = beta._data[ 0U ];
        penLocation._data[ 1U ] += beta._data[ 1U ];
    };

    switch ( _css._position )
    {
        case PositionProperty::eValue::Absolute:
            penLocation.Sum ( info._parentTopLeft, alpha );
        break;

        case PositionProperty::eValue::Static:
        {
            if ( !_isInlineBlock )
            {
                newLine ();
                break;
            }

            penLocation.Sum ( info._penLocation, alpha );
            float const beta = penLocation._data[ 0U ] + marginRight + paddingRight;

            if ( parentWidth + parentLeft - beta <= 0.0F )
            {
                newLine ();
            }
        }
        break;

        default:
            AV_ASSERT ( false )
        return;
    }

    GXVec2 const imageArea = ResolveSize ( canvasSize, units );

    if ( _css._position == PositionProperty::eValue::Absolute )
    {
        GXVec2 topLeftOffset {};
        _topLeft = penLocation;
        _bottomRight.Sum ( _topLeft, imageArea );
        return;
    }

    // 'static' position territory

    GXVec2 beta {};
    beta.Sum ( imageArea, alpha );

    GXVec2 gamma {};
    gamma.Sum ( marginBottomRight, paddingBottomRight );

    GXVec2 blockSize {};
    blockSize.Sum ( beta, gamma );

    if ( ( blockSize._data[ 0U ] <= 0.0F ) | ( blockSize._data[ 1U ] <= 0.0F ) )
        return;

    info._vertices += UIPass::GetVerticesPerRectangle ();

    auto const computeVisibleBounds = [ & ] () noexcept {
        // Reusing "marginBottomRight" and "marginTopLeft" variables. They will be not used anyway.
        marginTopLeft.Reverse ();
        marginBottomRight.Reverse ();

        GXVec2 yotta {};
        yotta.Sum ( blockSize, GXVec2 ( marginTopLeft._data[ 0U ], marginBottomRight._data[ 1U ] ) );

        _topLeft.Subtract ( info._penLocation, GXVec2 ( yotta._data[ 0U ], -marginTopLeft._data[ 1U ] ) );
        _bottomRight.Sum ( info._penLocation, GXVec2 ( marginBottomRight._data[ 0U ], yotta._data[ 1U ] ) );
    };

    if ( _css._display == DisplayProperty::eValue::Block )
    {
        // Block starts from new line and consumes whole parent block line.
        float const cases[] = { blockSize._data[ 1U ], info._currentLineHeight };
        auto const s = static_cast<size_t> ( info._currentLineHeight != 0.0F );

        info._currentLineHeight = cases[ s ];
        info._newLines = s;
        info._newLineHeight = blockSize._data[ 1U ];
        info._penLocation._data[ 0U ] = parentLeft + blockSize._data[ 0U ];
        info._penLocation._data[ 1U ] = info._parentTopLeft._data[ 1U ];

        computeVisibleBounds ();
        return;
    }

    // 'inline-block' territory.

    bool const firstBlock = info._penLocation.IsEqual ( info._parentTopLeft );
    float const rest = parentWidth + parentLeft - info._penLocation._data[ 0U ];
    bool const blockCanFit = rest >= blockSize._data[ 0U ];

    if ( firstBlock | blockCanFit )
    {
        info._currentLineHeight = std::max ( info._currentLineHeight, blockSize._data[ 1U ] );
        info._newLines = 0U;
        info._penLocation._data[ 0U ] += blockSize._data[ 0U ];

        computeVisibleBounds ();
        return;
    }

    // Block goes to the new line of parent block.
    info._newLines = 1U;
    info._newLineHeight = blockSize._data[ 1U ];
    info._penLocation._data[ 0U ] = parentLeft + blockSize._data[ 0U ];
    info._penLocation._data[ 1U ] += info._currentLineHeight;

    computeVisibleBounds ();
}

void ImageUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    constexpr GXColorRGB white ( 1.0F, 1.0F, 1.0F, 1.0F );
    constexpr GXVec2 imageTopLeft ( 0.0F, 0.0F );
    constexpr GXVec2 imageBottomRight ( 1.0F, 1.0F );

    FontStorage::GlyphInfo const& g = info._fontStorage->GetTransparentGlyphInfo ();
    UIVertexBuffer& vertexBuffer = info._vertexBuffer;

    UIPass::AppendRectangle ( vertexBuffer.data (),
        white,
        _topLeft,
        _bottomRight,
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
