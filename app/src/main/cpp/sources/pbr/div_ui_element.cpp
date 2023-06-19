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

    float const widthCases[] =
    {
        ResolvePixelLength ( _css._width, parentWidth, false, units ),
        parentWidth + parentLeft - ( penLocation._data[ 0U ] + paddingRight + marginRight )
    };

    GXVec2 canvas ( widthCases[ static_cast<size_t> ( _isAutoWidth | !_isInlineBlock ) ],
        ResolvePixelLength ( _css._height, canvasSize._data[ 1U ], true, units )
    );

    if ( canvas._data[ 0U ] == 0.0F )
    {
        // Trying to resolve recursion with 'auto' width from parent and 'percentage' width from child.
        bool const isPercent = _css._width.GetType () == LengthValue::eType::Percent;
        float const cases[] = { 0.0F, 1.0e-2F * parentWidth * _css._width.GetValue () };
        canvas._data[ 0U ] = cases[ static_cast<size_t> ( isPercent & !_isInlineBlock ) ];
    }

    ApplyLayoutInfo childInfo
    {
        ._canvasSize = canvas,
        ._cssUnits = info._cssUnits,
        ._currentLineHeight = 0.0F,
        ._fontStorage = info._fontStorage,
        ._newLineHeight = 0.0F,
        ._newLines = 0U,
        ._parentTopLeft = penLocation,
        ._penLocation = penLocation,
        ._renderer = info._renderer,
        ._vertices = 0U
    };

    _lineHeights.clear ();

    if ( canvas._data[ 0U ] > 0.0F  )
        ProcessChildren ( childInfo );

    bool const hasLines = !_lineHeights.empty ();

    if ( _isAutoHeight & hasLines )
        canvas._data[ 1U ] = childInfo._penLocation._data[ 1U ] - penLocation._data[ 1U ] + _lineHeights.back ();

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
    canvas._data[ 0U ] = finalWidth[ selector ];

    GXVec2 padding {};
    padding.Sum ( paddingTopLeft, paddingBottomRight );

    GXVec2 drawableArea {};
    drawableArea.Sum ( canvas, padding );

    auto const sizeCheck = [] ( GXVec2 const &size ) noexcept -> bool {
        return ( size._data[ 0U ] > 0.0F ) & ( size._data[ 1U ] > 0.0F );
    };

    // Opaque background requires rectangle (two triangles).
    size_t const vertices[] = { childInfo._vertices, childInfo._vertices + UIPass::GetVerticesPerRectangle () };
    bool const hasBackgroundColor = _css._backgroundColor.GetValue ()._data[ 3U ] != 0.0F;
    bool const hasBackgroundArea = sizeCheck ( drawableArea );
    info._vertices += vertices[ static_cast<size_t> ( hasBackgroundColor & hasBackgroundArea ) ];

    if ( _css._position == PositionProperty::eValue::Absolute )
    {
        _topLeft = marginTopLeft;
        _bottomRight.Sum ( marginTopLeft, drawableArea );
        return;
    }

    GXVec2 beta {};
    beta.Sum ( canvas, alpha );

    GXVec2 gamma {};
    gamma.Sum ( marginBottomRight, paddingBottomRight );

    GXVec2 blockSize {};
    blockSize.Sum ( beta, gamma );

    if ( !sizeCheck ( blockSize ) )
        return;

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

void DIVUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    GXColorRGB const& color = _css._backgroundColor.GetValue ();

    if ( color._data[ 3U ] > 0.0F )
    {
        constexpr GXVec2 imageUV ( 0.5F, 0.5F );

        UIVertexBuffer& vertexBuffer = info._vertexBuffer;
        FontStorage::GlyphInfo const& glyphInfo = info._fontStorage->GetOpaqueGlyphInfo ();

        UIPass::AppendRectangle ( vertexBuffer.data (),
            color,
            _topLeft,
            _bottomRight,
            glyphInfo._topLeft,
            glyphInfo._bottomRight,
            imageUV,
            imageUV
        );

        vertexBuffer = vertexBuffer.subspan ( UIPass::GetVerticesPerRectangle () );
        info._uiPass->SubmitRectangle ();
    }

    for ( auto* child : _children )
    {
        child->Submit ( info );
    }
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

void DIVUIElement::ProcessChildren ( ApplyLayoutInfo &childInfo ) noexcept
{
    _lineHeights.push_back ( 0.0F );

    for ( auto* child : _children )
    {
        child->ApplyLayout ( childInfo );
        _lineHeights.front () = childInfo._currentLineHeight;

        size_t const newLines = childInfo._newLines;

        if ( !newLines )
            continue;

        float const h = childInfo._newLineHeight;
        _lineHeights.reserve ( _lineHeights.size () + newLines );

        for ( size_t i = 0U; i < newLines; ++i )
            _lineHeights.push_back ( h );

        childInfo._currentLineHeight = h;
    }
}

float DIVUIElement::ResolvePixelLength ( LengthValue const &length,
    float parentLength,
    bool isHeight,
    CSSUnitToDevicePixel const &units
) const noexcept
{
    switch ( length.GetType () )
    {
        case LengthValue::eType::EM:
        return static_cast<float> ( ResolveFontSize ( units, *this ) );

        case LengthValue::eType::MM:
        return units._fromMM * length.GetValue ();

        case LengthValue::eType::PT:
        return units._fromPT * length.GetValue ();

        case LengthValue::eType::PX:
        return units._fromPX * length.GetValue ();

        case LengthValue::eType::Percent:
        {
            if ( _parent )
            {
                // NOLINTNEXTLINE - downcast.
                auto const& div = *static_cast<DIVUIElement const*> ( _parent );

                LengthValue const* cases[] = { &div._css._width, &div._css._height };
                bool const isAuto = cases[ static_cast<size_t> ( isHeight ) ]->GetType () == LengthValue::eType::Auto;

                if ( isAuto & isHeight )
                {
                    // It's recursion. Doing exactly the same as Google Chrome v114.0.5735.110 does.
                    return 0.0F;
                }
            }

            return 1.0e-2F * parentLength * length.GetValue ();
        }

        case LengthValue::eType::Auto:
        return 0.0F;

        default:
            AV_ASSERT ( false )
        return 0.0F;
    }
}

} // namespace pbr
