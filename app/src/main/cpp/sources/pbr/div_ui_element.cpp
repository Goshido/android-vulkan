#include <pbr/div_ui_element.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>

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
    CSSUIElement ( css._display != DisplayProperty::eValue::None, parent, std::move ( css ) ),
    _isAutoWidth ( _css._width.GetType () == LengthValue::eType::Auto ),
    _isAutoHeight ( _css._height.GetType () == LengthValue::eType::Auto ),
    _isInlineBlock ( _css._display == DisplayProperty::eValue::InlineBlock ),

    _widthSelectorBase (
        ( static_cast<size_t> ( _isInlineBlock ) << 2U ) | ( static_cast<size_t> ( _isAutoWidth ) << 1U )
    )
{
    _css._fontFile = std::move ( android_vulkan::File ( std::move ( _css._fontFile ) ).GetPath () );

    if ( success = lua_checkstack ( &vm, 2 ); !success ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterDIVUIElement" ) == LUA_TFUNCTION; !success ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::DIVUIElement::DIVUIElement - Can't append element inside Lua VM." );
    }
}

void DIVUIElement::ApplyLayout ( ApplyInfo &info ) noexcept
{
    if ( !_visible )
        return;

    // Method contains a lot of branchless optimizations.
    std::vector<float> &lineHeights = *info._lineHeights;
    _parentLine = lineHeights.size () - 1U;

    size_t const oldVertices = info._vertices;
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
            _parentLine = std::numeric_limits<size_t>::max ();
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

    float const &parentWidth = canvasSize._data[ 0U ];
    float const &paddingRight = paddingBottomRight._data[ 0U ];
    float const &marginRight = marginBottomRight._data[ 0U ];

    float const widthCases[] =
    {
        ResolvePixelLength ( _css._width, parentWidth, false ),
        parentWidth - ( pen._data[ 0U ] + paddingRight + marginRight )
    };

    _canvasSize = GXVec2 ( widthCases[ static_cast<size_t> ( _isAutoWidth | !_isInlineBlock ) ],
        ResolvePixelLength ( _css._height, canvasSize._data[ 1U ], true )
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

    ApplyInfo childInfo
    {
        ._canvasSize = _canvasSize,
        ._fontStorage = info._fontStorage,
        ._hasChanges = false,
        ._lineHeights = &_lineHeights,
        ._pen = GXVec2 ( 0.0F, 0.0F ),
        ._renderer = info._renderer,
        ._vertices = 0U
    };

    for ( auto* child : _children )
        child->ApplyLayout ( childInfo );

    bool const hasLines = !_lineHeights.empty ();

    if ( _isAutoHeight & hasLines )
        _canvasSize._data[ 1U ] = childInfo._pen._data[ 1U ] + _lineHeights.back ();

    float const finalWidth[] =
    {
        widthCases[ 0U ],
        widthCases[ 0U ],
        widthCases[ 1U ],
        widthCases[ 1U ],
        widthCases[ 0U ],
        widthCases[ 0U ],
        childInfo._pen._data[ 0U ],
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
    _hasBackground = hasBackgroundColor & hasBackgroundArea;
    info._vertices += vertices[ static_cast<size_t> ( _hasBackground ) ];

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
        float const h = cases[ static_cast<size_t> ( currentLineHeight != 0.0F ) ];

        lineHeights.back () = h;
        lineHeights.push_back ( 0.0F );

        penOut._data[ 0U ] = 0.0F;
        penOut._data[ 1U ] += h;

        info._hasChanges |= childInfo._hasChanges;
        return;
    }

    // 'inline-block' territory.

    constexpr GXVec2 zero ( 0.0F, 0.0F );
    bool const firstBlock = penOut.IsEqual ( zero );
    float const rest = parentWidth - penOut._data[ 0U ];
    bool const blockCanFit = rest >= _blockSize._data[ 0U ] - 0.25F;

    if ( firstBlock | blockCanFit )
    {
        float const h = std::max ( lineHeights.back (), _blockSize._data[ 1U ] );
        lineHeights.back () = h;
        penOut._data[ 0U ] += _blockSize._data[ 0U ];
        info._hasChanges |= childInfo._hasChanges;
        return;
    }

    // Block goes to the new line of parent block and requires recalculation.
    penOut._data[ 0U ] = 0.0F;
    penOut._data[ 1U ] += lineHeights.back ();
    lineHeights.push_back ( 0.0F );
    info._vertices = oldVertices;

    ApplyLayout ( info );
}

void DIVUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    if ( _hasBackground )
    {
        constexpr size_t vertices = UIPass::GetVerticesPerRectangle ();
        constexpr size_t bytes = vertices * sizeof ( UIVertexInfo );

        UIVertexBuffer &uiVertexBuffer = info._vertexBuffer;
        std::memcpy ( uiVertexBuffer.data (), _vertices, bytes );
        uiVertexBuffer = uiVertexBuffer.subspan ( vertices );

        info._uiPass->SubmitRectangle ();
    }

    for ( auto* child : _children )
    {
        child->Submit ( info );
    }
}

bool DIVUIElement::UpdateCache ( UpdateInfo &info ) noexcept
{
    bool needRefill = _visibilityChanged;
    _visibilityChanged = false;

    if ( !_visible )
        return needRefill;

    GXVec2 pen {};

    if ( _css._position == PositionProperty::eValue::Static )
    {
        pen = info._pen;

        if ( info._line != _parentLine )
        {
            pen._data[ 0U ] = info._parentTopLeft._data[ 0U ];
            pen._data[ 1U ] += info._parentLineHeights[ info._line ];

            info._pen._data[ 1U ] = pen._data[ 1U ];
            info._line = _parentLine;
        }
    }
    else
    {
        // 'absolute' block territory.
        pen = info._parentTopLeft;

        if ( _css._top.GetType () != LengthValue::eType::Auto )
            pen._data[ 1U ] += ResolvePixelLength ( _css._top, info._parentSize._data[ 1U ], true );

        if ( _css._bottom.GetType () != LengthValue::eType::Auto )
        {
            float const offset = ResolvePixelLength ( _css._bottom,
                info._parentSize._data[ 1U ],
                true
            );

            pen._data[ 1U ] += info._parentSize._data[ 1U ] - ( _blockSize._data[ 1U ] + offset );
        }

        if ( _css._left.GetType () != LengthValue::eType::Auto )
            pen._data[ 0U ] += ResolvePixelLength ( _css._left, info._parentSize._data[ 0U ], false );

        if ( _css._right.GetType () != LengthValue::eType::Auto )
        {
            float const offset = ResolvePixelLength ( _css._right,
                info._parentSize._data[ 0U ],
                false
            );

            pen._data[ 0U ] += info._parentSize._data[ 0U ] - ( _blockSize._data[ 0U ] + offset );
        }
    }

    AlignHander const verticalAlign = ResolveVerticalAlignment ( static_cast<CSSUIElement const &> ( *this ) );
    pen._data[ 1U ] = verticalAlign ( pen._data[ 1U ], info._parentLineHeights[ _parentLine ], _blockSize._data[ 1U ] );

    if ( _hasBackground )
    {
        GXColorRGB const &color = _css._backgroundColor.GetValue ();
        constexpr GXVec2 imageUV ( 0.5F, 0.5F );

        GXVec2 topLeft {};
        topLeft.Sum ( pen, _marginTopLeft );

        GXVec2 bottomRight {};
        bottomRight.Sum ( topLeft, _borderSize );

        FontStorage::GlyphInfo const &glyphInfo = info._fontStorage->GetOpaqueGlyphInfo ();

        UIPass::AppendRectangle ( _vertices,
            color,
            topLeft,
            bottomRight,
            glyphInfo._topLeft,
            glyphInfo._bottomRight,
            imageUV,
            imageUV
        );
    }

    GXVec2 topLeft {};
    topLeft.Sum ( pen, _canvasTopLeftOffset );

    UpdateInfo updateInfo
    {
        ._fontStorage = info._fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = _canvasSize,
        ._parentTopLeft = topLeft,
        ._pen = topLeft
    };

    for ( auto* child : _children )
        needRefill |= child->UpdateCache ( updateInfo );

    info._pen._data[ 0U ] += _blockSize._data[ 0U ];
    return needRefill;
}

bool DIVUIElement::AppendChildElement ( lua_State &vm,
    int errorHandlerIdx,
    int appendChildElementIdx,
    UIElement &element
) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::AppendChildElement - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, appendChildElementIdx );
    lua_pushvalue ( &vm, -3 );
    lua_rotate ( &vm, -3, -1 );

    if ( lua_pcall ( &vm, 2, 0, errorHandlerIdx ) == LUA_OK ) [[likely]]
    {
        _children.emplace_back ( &element );
        return true;
    }

    android_vulkan::LogWarning ( "pbr::DIVUIElement::AppendChildElement - Can't append child element inside Lua VM." );
    return false;
}

} // namespace pbr
