#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <platform/android/pbr/div_ui_element.hpp>


namespace pbr {

DIVUIElement::DIVUIElement ( UIElement const* parent, CSSComputedValues &&css ) noexcept:
    UIElement ( css._display != DisplayProperty::eValue::None, parent, std::move ( css ) )
{
    _css._fontFile = std::move ( android_vulkan::File ( std::move ( _css._fontFile ) ).GetPath () );
}

DIVUIElement::DIVUIElement ( UIElement const* parent, CSSComputedValues &&css, std::string &&name ) noexcept:
    UIElement ( css._display != DisplayProperty::eValue::None, parent, std::move ( css ), std::move ( name ) )
{
    _css._fontFile = std::move ( android_vulkan::File ( std::move ( _css._fontFile ) ).GetPath () );
}

void DIVUIElement::ApplyLayout ( ApplyInfo &info ) noexcept
{
    info._hasChanges |= std::exchange ( _hasChanges, false );

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
    bool const isInlineBlock = _css._display == DisplayProperty::eValue::InlineBlock;
    GXVec2 referenceSize = canvasSize;

    switch ( _css._position )
    {
        case PositionProperty::eValue::Absolute:
            pen = _canvasTopLeftOffset;
            _parentLine = std::numeric_limits<size_t>::max ();

            // Reference size should include parent padding for absolute positioned elements.
            // https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_display/Containing_block#identifying_the_containing_block
            referenceSize.Sum ( referenceSize, info._parentPaddingExtent );
        break;

        case PositionProperty::eValue::Relative:
            [[fallthrough]];
        case PositionProperty::eValue::Static:
        {
            if ( penOut._data[ 0U ] == 0.0F )
            {
                pen.Sum ( penOut, _canvasTopLeftOffset );
                break;
            }

            if ( !isInlineBlock )
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

    float const &parentWidth = referenceSize._data[ 0U ];
    float const &paddingRight = paddingBottomRight._data[ 0U ];
    float const &marginRight = marginBottomRight._data[ 0U ];

    float const widthCases[] =
    {
        ResolvePixelLength ( _css._width, parentWidth, false ),
        parentWidth - ( pen._data[ 0U ] + paddingRight + marginRight )
    };

    bool const isAutoWidth = _css._width.GetType () == LengthValue::eType::Auto;
    auto selector = static_cast<size_t> ( isAutoWidth | ( _css._display == DisplayProperty::eValue::None ) );

    _canvasSize = GXVec2 ( widthCases[ selector ],
        ResolvePixelLength ( _css._height, referenceSize._data[ 1U ], true )
    );

    if ( _canvasSize._data[ 0U ] == 0.0F )
    {
        // Trying to resolve recursion with 'auto' width from parent and 'percentage' width from child.
        bool const isPercent = _css._width.GetType () == LengthValue::eType::Percent;
        float const cases[] = { 0.0F, 1.0e-2F * parentWidth * _css._width.GetValue () };
        _canvasSize._data[ 0U ] = cases[ static_cast<size_t> ( isPercent & !isInlineBlock ) ];
    }

    _lineHeights.clear ();
    _lineHeights.push_back ( 0.0F );

    GXVec2 parentPaddingExtent ( paddingTopLeft );
    parentPaddingExtent.Sum ( parentPaddingExtent, paddingBottomRight );

    ApplyInfo childInfo
    {
        ._canvasSize = _canvasSize,
        ._fontStorage = info._fontStorage,
        ._hasChanges = false,
        ._lineHeights = &_lineHeights,
        ._parentPaddingExtent = parentPaddingExtent,
        ._pen = GXVec2 ( 0.0F, 0.0F ),
        ._renderer = info._renderer,
        ._vertices = 0U
    };

    for ( auto* child : _children )
        child->ApplyLayout ( childInfo );

    bool const hasLines = !_lineHeights.empty ();

    if ( ( _css._height.GetType () == LengthValue::eType::Auto ) & hasLines )
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

    selector = ( static_cast<size_t> ( isInlineBlock ) << 2U ) |
        ( static_cast<size_t> ( isAutoWidth ) << 1U ) |
        static_cast<size_t> ( _lineHeights.size () > 1U );

    _canvasSize._data[ 0U ] = finalWidth[ selector ];

    GXVec2 padding {};
    padding.Sum ( paddingTopLeft, paddingBottomRight );

    _borderSize.Sum ( _canvasSize, padding );

    constexpr auto sizeCheck = [] ( GXVec2 const &size ) noexcept -> bool {
        return ( size._data[ 0U ] > 0.0F ) & ( size._data[ 1U ] > 0.0F );
    };

    // Opaque background requires rectangle (two triangles).

    size_t const vertices[] =
    {
        childInfo._vertices,
        childInfo._vertices + UIPass::GetVerticesPerRectangle ()
    };

    bool const hasBackgroundColor = _css._backgroundColor.GetSRGB()._data[ 3U ] != 0U;
    bool const hasBackgroundArea = sizeCheck ( _borderSize );
    _hasBackground = hasBackgroundColor & hasBackgroundArea;
    info._vertices += vertices[ static_cast<size_t> ( _hasBackground ) ];

    GXVec2 beta {};
    beta.Sum ( _canvasSize, _canvasTopLeftOffset );

    GXVec2 gamma {};
    gamma.Sum ( marginBottomRight, paddingBottomRight );

    _blockSize.Sum ( beta, gamma );

    if ( _css._position == PositionProperty::eValue::Absolute )
    {
        info._hasChanges |= childInfo._hasChanges;
        return;
    }

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
    // Based on ideas from https://iamvdo.me/en/blog/css-font-metrics-line-height-and-vertical-align
    // The most significant feature - adding implicit spaces between blocks in horizontal and vertical direction.
    // [2025-01-06] It was discovered that Google Chrome v131.0.6778.205 is using actual space (U+0020) character
    // of current font family and current font size to get horizontal spacing value. The kerning is ignored.
    // For vertical spacing the following properties must be considered:
    //  - current font family
    //  - current font size
    //  - current line-height

    // FUCK needed to implement

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
        constexpr size_t positionBytes = vertices * sizeof ( GXVec2 );
        constexpr size_t verticesBytes = vertices * sizeof ( UIVertex );

        UIBufferStreams &uiVertexBuffer = info._uiBufferStreams;
        std::span<GXVec2> &uiPositions = uiVertexBuffer._positions;
        std::span<UIVertex> &uiVertices = uiVertexBuffer._vertices;

        std::memcpy ( uiPositions.data (), _positions, positionBytes );
        std::memcpy ( uiVertices.data (), _vertices, verticesBytes );

        uiPositions = uiPositions.subspan ( vertices );
        uiVertices = uiVertices.subspan ( vertices );

        info._uiPass->SubmitRectangle ();
    }

    for ( auto* child : _children )
    {
        child->Submit ( info );
    }
}

bool DIVUIElement::UpdateCache ( UpdateInfo &info ) noexcept
{
    bool needRefill = std::exchange ( _visibilityChanged, false );

    if ( !_visible )
        return needRefill;

    GXVec2 pen {};
    AlignHandler verticalAlign = &UIElement::AlignToStart;
    float parentHeight = 0.0F;
    PositionProperty::eValue const position = _css._position;

    if ( ( position == PositionProperty::eValue::Static ) | ( position == PositionProperty::eValue::Relative ) )
    {
        pen = info._pen;
        verticalAlign = ResolveVerticalAlignment ( *this );
        parentHeight = info._parentLineHeights[ _parentLine ];

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

        for ( UIElement const* p = _parent; p; p = p->_parent )
        {
            // According to CSS calculations should be done relative Nearest Positioned Ancestor.
            // Long story short: it's needed to work relative closest 'position: absolute or relative' element.
            if ( p->GetCSS ()._position == PositionProperty::eValue::Static )
                continue;

            // NOLINTNEXTLINE - downcast
            pen = static_cast<DIVUIElement const*> ( p )->_absoluteRect._topLeft;
            break;
        }

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

    pen._data[ 1U ] = verticalAlign ( pen._data[ 1U ], parentHeight, _blockSize._data[ 1U ] );

    _absoluteRect._topLeft.Sum ( pen, _marginTopLeft );
    _absoluteRect._bottomRight.Sum ( _absoluteRect._topLeft, _borderSize );

    if ( _hasBackground )
    {
        UIPass::AppendRectangle ( _positions,
            _vertices,
            _css._backgroundColor.GetSRGB (),
            _absoluteRect._topLeft,
            _absoluteRect._bottomRight
        );
    }

    GXVec2 topLeft {};
    topLeft.Sum ( pen, _canvasTopLeftOffset );

    auto fontProbe = info._fontStorage->GetFont ( ResolveFont (), static_cast<uint32_t> ( ResolveFontSize () ) );

    if ( !fontProbe ) [[unlikely]]
        return needRefill;

    UpdateInfo updateInfo
    {
        ._fontStorage = info._fontStorage,
        ._line = 0U,
        ._lineHeight = ResolveLineHeight ( fontProbe->_font ),
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = _canvasSize,
        ._parentTopLeft = topLeft,
        ._pen = topLeft
    };

    fontProbe = std::nullopt;

    for ( auto* child : _children )
        needRefill |= child->UpdateCache ( updateInfo );

    info._pen._data[ 0U ] += _blockSize._data[ 0U ];
    return needRefill;
}

void DIVUIElement::AppendChildElement ( UIElement &element ) noexcept
{
    _children.emplace_back ( &element );
}

void DIVUIElement::PrependChildElement ( UIElement& element ) noexcept
{
    _children.emplace_front ( &element );
}

DIVUIElement::Rect const &DIVUIElement::GetAbsoluteRect () const noexcept
{
    return _absoluteRect;
}

void DIVUIElement::Update () noexcept
{
    _hasChanges = true;
}

} // namespace pbr
