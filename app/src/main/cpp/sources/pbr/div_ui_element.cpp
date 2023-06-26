#include <pbr/div_ui_element.h>
#include <av_assert.h>
#include <file.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

DIVUIElement::DIVUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    CSSComputedValues &&css
) noexcept:
    UIElement ( css._display != DisplayProperty::eValue::None, parent ),
    _isAutoWidth ( css._width.GetType () == LengthValue::eType::Auto ),
    _isAutoHeight ( css._height.GetType () == LengthValue::eType::Auto ),
    _isInlineBlock ( css._display == DisplayProperty::eValue::InlineBlock ),

    _widthSelectorBase (
        ( static_cast<size_t> ( _isInlineBlock ) << 2U ) | ( static_cast<size_t> ( _isAutoWidth ) << 1U )
    ),

    _css ( std::move ( css ) )
{
    _css._fontFile = std::move ( android_vulkan::File ( std::move ( _css._fontFile ) ).GetPath () );

    if ( success = lua_checkstack ( &vm, 2 ); !success )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterDIVUIElement" ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success )
    {
        android_vulkan::LogWarning ( "pbr::DIVUIElement::DIVUIElement - Can't append element inside Lua VM." );
    }
}

void DIVUIElement::ApplyLayout ( ApplyLayoutInfo &info ) noexcept
{
    if ( !_visible )
        return;

    // Method contains a lot of branchless optimizations.
    std::vector<float>& lineHeights = *info._lineHeights;
    _parentLine = lineHeights.size () - 1U;
    size_t const oldVertices = info._vertices;

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

    auto const newLine = [ & ] () noexcept {
        penLocation._data[ 0U ] = _canvasTopLeftOffset._data[ 0U ];
        penLocation._data[ 1U ] += _canvasTopLeftOffset._data[ 1U ] + lineHeights.back ();
        ++_parentLine;
    };

    GXVec2& outPenLocation = info._penLocation;

    switch ( _css._position )
    {
        case PositionProperty::eValue::Absolute:
            penLocation = _canvasTopLeftOffset;
            _parentLine = 0U;
        break;

        case PositionProperty::eValue::Static:
        {
            if ( outPenLocation._data[ 0U ] == 0.0F )
            {
                penLocation.Sum ( outPenLocation, _canvasTopLeftOffset );
                break;
            }

            if ( !_isInlineBlock )
            {
                newLine ();
                break;
            }

            penLocation.Sum ( outPenLocation, _canvasTopLeftOffset );
        }
        break;

        default:
            AV_ASSERT ( false )
        return;
    }

    float const& parentWidth = canvasSize._data[ 0U ];
    float const& paddingRight = paddingBottomRight._data[ 0U ];
    float const& marginRight = marginBottomRight._data[ 0U ];

    float const widthCases[] =
    {
        ResolvePixelLength ( _css._width, parentWidth, false, units ),
        parentWidth - ( penLocation._data[ 0U ] + paddingRight + marginRight )
    };

    _canvasSize = GXVec2 ( widthCases[ static_cast<size_t> ( _isAutoWidth | !_isInlineBlock ) ],
        ResolvePixelLength ( _css._height, canvasSize._data[ 1U ], true, units )
    );

    if ( _canvasSize._data[ 0U ] == 0.0F )
    {
        // Trying to resolve recursion with 'auto' width from parent and 'percentage' width from child.
        bool const isPercent = _css._width.GetType () == LengthValue::eType::Percent;
        float const cases[] = { 0.0F, 1.0e-2F * parentWidth * _css._width.GetValue () };
        _canvasSize._data[ 0U ] = cases[ static_cast<size_t> ( isPercent & !_isInlineBlock ) ];
    }

    _lineHeights.clear ();
    _lineHeights.push_back ( 0.0F );

    ApplyLayoutInfo childInfo
    {
        ._canvasSize = _canvasSize,
        ._cssUnits = info._cssUnits,
        ._fontStorage = info._fontStorage,
        ._lineHeights = &_lineHeights,
        ._penLocation = GXVec2 ( 0.0F, 0.0F ),
        ._renderer = info._renderer,
        ._vertices = 0U
    };

    for ( auto* child : _children )
        child->ApplyLayout ( childInfo );

    bool const hasLines = !_lineHeights.empty ();

    if ( _isAutoHeight & hasLines )
        _canvasSize._data[ 1U ] = childInfo._penLocation._data[ 1U ] + _lineHeights.back ();

    float const finalWidth[] =
    {
        widthCases[ 0U ],
        widthCases[ 0U ],
        widthCases[ 1U ],
        widthCases[ 1U ],
        widthCases[ 0U ],
        widthCases[ 0U ],
        childInfo._penLocation._data[ 0U ] - penLocation._data[ 0U ],
        widthCases[ 1U ]
    };

    bool const isFullLine = _lineHeights.size () > 1U;
    size_t const selector = _widthSelectorBase | static_cast<size_t> ( isFullLine );
    _canvasSize._data[ 0U ] = finalWidth[ selector ];

    GXVec2 padding {};
    padding.Sum ( paddingTopLeft, paddingBottomRight );

    _borderSize.Sum ( _canvasSize, padding );

    auto const sizeCheck = [] ( GXVec2 const &size ) noexcept -> bool {
        return ( size._data[ 0U ] > 0.0F ) & ( size._data[ 1U ] > 0.0F );
    };

    // Opaque background requires rectangle (two triangles).
    size_t const vertices[] = { childInfo._vertices, childInfo._vertices + UIPass::GetVerticesPerRectangle () };
    bool const hasBackgroundColor = _css._backgroundColor.GetValue ()._data[ 3U ] != 0.0F;
    bool const hasBackgroundArea = sizeCheck ( _borderSize );
    info._vertices += vertices[ static_cast<size_t> ( hasBackgroundColor & hasBackgroundArea ) ];

    GXVec2 beta {};
    beta.Sum ( _canvasSize, _canvasTopLeftOffset );

    GXVec2 gamma {};
    gamma.Sum ( marginBottomRight, paddingBottomRight );

    _blockSize.Sum ( beta, gamma );

    if ( _css._position == PositionProperty::eValue::Absolute )
        return;

    if ( !sizeCheck ( _blockSize ) )
        return;

    if ( _css._display == DisplayProperty::eValue::Block )
    {
        // Block starts from new line and consumes whole parent block line.
        float const currentLineHeight = lineHeights.back ();

        float const cases[] = { _blockSize._data[ 1U ], currentLineHeight };
        auto const s = static_cast<size_t> ( currentLineHeight != 0.0F );

        lineHeights.back () = cases[ s ];
        outPenLocation._data[ 0U ] = 0.0F;
        outPenLocation._data[ 1U ] += cases[ s ];
        return;
    }

    // 'inline-block' territory.

    bool const firstBlock = outPenLocation._data[ 0U ] == 0.0F & outPenLocation._data[ 1U ] == 0.0F;
    float const rest = parentWidth - outPenLocation._data[ 0U ];
    bool const blockCanFit = rest >= _blockSize._data[ 0U ] - 0.25F;

    if ( firstBlock | blockCanFit )
    {
        lineHeights.back () = std::max ( lineHeights.back (), _blockSize._data[ 1U ] );
        outPenLocation._data[ 0U ] += _blockSize._data[ 0U ];
        return;
    }

    // Block goes to the new line of parent block and requires recalculation.
    outPenLocation._data[ 0U ] = 0.0F;
    outPenLocation._data[ 1U ] += lineHeights.back ();
    info._lineHeights->push_back ( 0.0F );
    info._vertices = oldVertices;

    ApplyLayout ( info );
}

void DIVUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    GXColorRGB const& color = _css._backgroundColor.GetValue ();
    GXVec2 pen = info._pen;

    if ( info._line != _parentLine )
    {
        pen._data[ 0U ] = info._parentTopLeft._data[ 0U ];
        pen._data[ 1U ] += info._parentLineHeights[ info._line ];
    }

    AlignHander const verticalAlign = ResolveVerticalAlignment ( this );
    pen._data[ 1U ] = verticalAlign ( pen._data[ 1U ], info._parentLineHeights[ _parentLine ], _blockSize._data[ 1U ] );

    if ( color._data[ 3U ] > 0.0F )
    {
        constexpr GXVec2 imageUV ( 0.5F, 0.5F );

        GXVec2 topLeft {};
        topLeft.Sum ( pen, _marginTopLeft );

        GXVec2 bottomRight {};
        bottomRight.Sum ( topLeft, _borderSize );

        UIVertexBuffer& vertexBuffer = info._vertexBuffer;
        FontStorage::GlyphInfo const& glyphInfo = info._fontStorage->GetOpaqueGlyphInfo ();

        UIPass::AppendRectangle ( vertexBuffer.data (),
            color,
            topLeft,
            bottomRight,
            glyphInfo._topLeft,
            glyphInfo._bottomRight,
            imageUV,
            imageUV
        );

        vertexBuffer = vertexBuffer.subspan ( UIPass::GetVerticesPerRectangle () );
        info._uiPass->SubmitRectangle ();
    }

    GXVec2 topLeft {};
    topLeft.Sum ( pen, _canvasTopLeftOffset );

    SubmitInfo submitInfo
    {
        ._fontStorage = info._fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentTopLeft = topLeft,
        ._parentWidth = _canvasSize._data[ 0U ],
        ._pen = topLeft,
        ._uiPass = info._uiPass,
        ._vertexBuffer = info._vertexBuffer
    };

    for ( auto* child : _children )
        child->Submit ( submitInfo );

    info._pen._data[ 0U ] += _blockSize._data[ 0U ];
    info._vertexBuffer = submitInfo._vertexBuffer;
}

bool DIVUIElement::AppendChildElement ( lua_State &vm,
    int errorHandlerIdx,
    int appendChildElementIdx,
    UIElement &element
) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::AppendChildElement - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, appendChildElementIdx );
    lua_pushvalue ( &vm, -3 );
    lua_rotate ( &vm, -3, -1 );

    if ( lua_pcall ( &vm, 2, 0, errorHandlerIdx ) == LUA_OK )
    {
        _children.emplace_back ( &element );
        return true;
    }

    android_vulkan::LogWarning ( "pbr::DIVUIElement::AppendChildElement - Can't append child element inside Lua VM." );
    return false;
}

} // namespace pbr
