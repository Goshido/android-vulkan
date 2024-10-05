#include <pbr/image_ui_element.hpp>
#include <av_assert.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

bool ImageUIElement::ApplyLayoutCache::Run ( ApplyInfo &info ) noexcept
{
    if ( _lineHeights.empty () )
    {
        // Worst case scenario.
        _lineHeights.reserve ( 2U );
        _lineHeights.push_back ( info._lineHeights->back () );
        return false;
    }

    bool const c0 = _secondIteration;
    bool const c1 = _lineHeights.front () < info._lineHeights->back ();
    bool const c2 = !_penIn.IsEqual ( info._pen );

    _secondIteration = false;

    if ( c0 | c1 | c2 )
    {
        if ( !c0 )
        {
            _lineHeights.clear ();
            _lineHeights.push_back ( info._lineHeights->back () );
        }

        return false;
    }

    info._pen = _penOut;
    info._vertices += UIPass::GetVerticesPerRectangle ();

    std::vector<float> &lines = *info._lineHeights;
    lines.pop_back ();
    lines.insert ( lines.cend (), _lineHeights.cbegin (), _lineHeights.cend () );

    return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool ImageUIElement::SubmitCache::Run ( UpdateInfo &info, std::vector<float> const &cachedLineHeight ) const noexcept
{
    bool const c0 = !_penIn.IsEqual ( info._pen );
    bool const c1 = !_parenTopLeft.IsEqual ( info._parentTopLeft );

    std::span<float const> dst ( info._parentLineHeights + info._line, cachedLineHeight.size () );
    bool const c2 = !std::equal ( dst.begin (), dst.end (), cachedLineHeight.cbegin () );

    if ( c0 | c1 | c2 )
        return false;

    info._pen = _penOut;
    return true;
}

//----------------------------------------------------------------------------------------------------------------------

ImageUIElement::ImageUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    std::string &&asset,
    CSSComputedValues &&css
) noexcept:
    CSSUIElement ( true, parent, std::move ( css ) ),
    _asset ( std::move ( asset ) ),
    _isAutoWidth ( _css._width.GetType () == LengthValue::eType::Auto ),
    _isAutoHeight ( _css._height.GetType () == LengthValue::eType::Auto ),
    _isInlineBlock ( _css._display == DisplayProperty::eValue::InlineBlock )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterImageUIElement" ) == LUA_TFUNCTION; !success ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ImageUIElement::ImageUIElement - Can't append element inside Lua VM." );
        return;
    }

    auto const texture = UIPass::RequestImage ( _asset );

    if ( success = texture.has_value (); success )
    {
        _submitCache._texture = *texture;
    }
}

ImageUIElement::~ImageUIElement () noexcept
{
    if ( _submitCache._texture )
    {
        UIPass::ReleaseImage ( _submitCache._texture );
    }
}

void ImageUIElement::ApplyLayout ( ApplyInfo &info ) noexcept
{
    if ( !_visible || _applyLayoutCache.Run ( info ) )
        return;

    // Method contains a lot of branchless optimizations.
    std::vector<float> &lineHeights = *info._lineHeights;
    _parentLine = lineHeights.size () - 1U;
    _applyLayoutCache._penIn = info._pen;

    _parentSize = info._canvasSize;
    GXVec2 const &canvasSize = info._canvasSize;

    _marginTopLeft = GXVec2 ( ResolvePixelLength ( _css._marginLeft, canvasSize._data[ 0U ], false ),
        ResolvePixelLength ( _css._marginTop, canvasSize._data[ 1U ], true )
    );

    GXVec2 const paddingTopLeft ( ResolvePixelLength ( _css._paddingLeft, canvasSize._data[ 0U ], false ),
        ResolvePixelLength ( _css._paddingTop, canvasSize._data[ 1U ], true )
    );

    GXVec2 marginBottomRight ( ResolvePixelLength ( _css._marginRight, canvasSize._data[ 0U ], false ),
        ResolvePixelLength ( _css._marginBottom, canvasSize._data[ 1U ], true )
    );

    GXVec2 const paddingBottomRight ( ResolvePixelLength ( _css._paddingRight, canvasSize._data[ 0U ], false ),
        ResolvePixelLength ( _css._paddingBottom, canvasSize._data[ 1U ], true )
    );

    _canvasTopLeftOffset.Sum ( _marginTopLeft, paddingTopLeft );

    GXVec2 pen {};
    float const &parentWidth = canvasSize._data[ 0U ];
    size_t const oldVertices = info._vertices;

    auto const newLine = [ & ] () noexcept {
        pen._data[ 0U ] = _canvasTopLeftOffset._data[ 0U ];
        pen._data[ 1U ] += _canvasTopLeftOffset._data[ 1U ] + lineHeights.back ();
        ++_parentLine;
    };

    GXVec2 &penOut = info._pen;

    switch ( _css._position )
    {
        case PositionProperty::eValue::Absolute:
            pen = _canvasTopLeftOffset;
            _parentLine = 0U;
        break;

        case PositionProperty::eValue::Static:
        {
            if ( penOut._data[ 0U ] == 0.0F )
            {
                pen.Sum ( penOut, _canvasTopLeftOffset );
                break;
            }

            if ( !_isInlineBlock )
            {
                newLine ();
                break;
            }

            pen.Sum ( penOut, _canvasTopLeftOffset );
        }
        break;

        default:
            AV_ASSERT ( false )
        return;
    }

    _borderSize = ResolveSize ( canvasSize );
    GXVec2 beta {};
    beta.Sum ( _borderSize, _canvasTopLeftOffset );

    GXVec2 gamma {};
    gamma.Sum ( marginBottomRight, paddingBottomRight );

    _blockSize.Sum ( beta, gamma );

    if ( _css._position == PositionProperty::eValue::Absolute )
    {
        info._hasChanges = true;
        _applyLayoutCache._lineHeights.back () = _blockSize._data[ 1U ];
        _applyLayoutCache._penOut = penOut;
        return;
    }

    // 'static' position territory

    info._vertices += UIPass::GetVerticesPerRectangle ();

    if ( _css._display == DisplayProperty::eValue::Block )
    {
        // Block starts from new line and consumes whole parent block line.
        float const currentLineHeight = lineHeights.back ();

        float const cases[] = { _blockSize._data[ 1U ], currentLineHeight };
        float const h = cases[ static_cast<size_t> ( currentLineHeight != 0.0F ) ];

        lineHeights.back () = h;
        _applyLayoutCache._lineHeights.back () = h;

        penOut._data[ 0U ] = 0.0F;
        penOut._data[ 1U ] += h;

        _applyLayoutCache._penOut = penOut;
        info._hasChanges = true;
        return;
    }

    // 'inline-block' territory.

    constexpr GXVec2 zero ( 0.0F, 0.0F );
    bool const firstBlock = penOut.IsEqual ( zero );
    float const rest = parentWidth - penOut._data[ 0U ];
    bool const blockCanFit = rest >= _blockSize._data[ 0U ];

    if ( firstBlock | blockCanFit )
    {
        float const h = std::max ( lineHeights.back (), _blockSize._data[ 1U ] );
        lineHeights.back () = h;
        _applyLayoutCache._lineHeights.back () = h;

        penOut._data[ 0U ] += _blockSize._data[ 0U ];
        _applyLayoutCache._penOut = penOut;

        info._hasChanges = true;
        return;
    }

    // Block goes to the new line of parent block.
    penOut._data[ 0U ] = 0.0F;
    penOut._data[ 1U ] += lineHeights.back ();
    lineHeights.push_back ( 0.0F );
    info._vertices = oldVertices;
    _applyLayoutCache._secondIteration = true;

    ApplyLayout ( info );
}

void ImageUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    constexpr size_t vertices = UIPass::GetVerticesPerRectangle ();
    constexpr size_t bytes = vertices * sizeof ( UIVertexInfo );

    UIVertexBuffer &uiVertexBuffer = info._vertexBuffer;
    std::memcpy ( uiVertexBuffer.data (), _submitCache._vertices, bytes );
    uiVertexBuffer = uiVertexBuffer.subspan ( vertices );

    info._uiPass->SubmitImage ( _submitCache._texture );
}

bool ImageUIElement::UpdateCache ( UpdateInfo &info ) noexcept
{
    bool const needRefill = _visibilityChanged;
    _visibilityChanged = false;

    if ( !_visible || _submitCache.Run ( info, _applyLayoutCache._lineHeights ) )
        return needRefill;

    _submitCache._parenTopLeft = info._parentTopLeft;
    _submitCache._penIn = info._pen;

    GXVec2 pen = info._pen;

    if ( info._line != _parentLine )
    {
        pen._data[ 0U ] = info._parentTopLeft._data[ 0U ];
        pen._data[ 1U ] += info._parentLineHeights[ info._line ];
    }

    AlignHander const verticalAlign = ResolveVerticalAlignment ( *this );
    pen._data[ 1U ] = verticalAlign ( pen._data[ 1U ], info._parentLineHeights[ _parentLine ], _blockSize._data[ 1U ] );

    GXVec2 topLeft {};
    topLeft.Sum ( pen, _canvasTopLeftOffset );

    float const &penX = info._pen._data[ 0U ];
    float const &borderOffsetX = _canvasTopLeftOffset._data[ 0U ];
    float const &parentLeft = _parentSize._data[ 0U ];
    float const &blockWidth = _blockSize._data[ 0U ];

    // NOLINTNEXTLINE - downcast.
    AlignHander const handler = ResolveTextAlignment ( static_cast<CSSUIElement const  &> ( *_parent ) );
    topLeft._data[ 0U ] = handler ( penX, parentLeft, blockWidth ) + borderOffsetX;

    GXVec2 bottomRight {};
    bottomRight.Sum ( topLeft, _borderSize );

    constexpr GXColorRGB white ( 1.0F, 1.0F, 1.0F, 1.0F );
    constexpr GXVec2 imageTopLeft ( 0.0F, 0.0F );
    constexpr GXVec2 imageBottomRight ( 1.0F, 1.0F );

    FontStorage::GlyphInfo const &g = info._fontStorage->GetTransparentGlyphInfo ();

    UIPass::AppendRectangle ( _submitCache._vertices,
        white,
        topLeft,
        bottomRight,
        g._topLeft,
        g._bottomRight,
        imageTopLeft,
        imageBottomRight
    );

    pen._data[ 0U ] += blockWidth;
    _submitCache._penOut = pen;

    info._pen = pen;
    return true;
}

GXVec2 ImageUIElement::ResolveSize ( GXVec2 const &parentCanvasSize ) noexcept
{
    if ( !_isAutoWidth & _isAutoHeight )
        return ResolveSizeByWidth ( parentCanvasSize._data[ 0U ] );

    if ( _isAutoWidth & !_isAutoHeight )
        return ResolveSizeByHeight ( parentCanvasSize._data[ 1U ] );

    return GXVec2 ( ResolvePixelLength ( _css._width, parentCanvasSize._data[ 0U ], false ),
        ResolvePixelLength ( _css._height, parentCanvasSize._data[ 1U ], true )
    );
}

GXVec2 ImageUIElement::ResolveSizeByWidth ( float parentWidth ) noexcept
{
    VkExtent2D const &r = _submitCache._texture->GetResolution ();

    GXVec2 result {};
    result._data[ 0U ] = ResolvePixelLength ( _css._width, parentWidth, false );
    result._data[ 1U ] = result._data[ 0U ] * static_cast<float> ( r.height ) / static_cast<float> ( r.width );

    return result;
}

GXVec2 ImageUIElement::ResolveSizeByHeight ( float parentHeight ) noexcept
{
    VkExtent2D const &r = _submitCache._texture->GetResolution ();

    GXVec2 result {};
    result._data[ 1U ] = ResolvePixelLength ( _css._height, parentHeight, true );
    result._data[ 0U ] = result._data[ 1U ] * static_cast<float> ( r.width ) / static_cast<float> ( r.height );

    return result;
}

} // namespace pbr
