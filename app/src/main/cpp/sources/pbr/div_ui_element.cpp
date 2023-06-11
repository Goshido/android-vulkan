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

    GXVec2 const& canvasSize = info._canvasSize;
    CSSUnitToDevicePixel const& units = *info._cssUnits;

    GXVec2 const marginTopLeft ( ResolvePixelLength ( _css._marginLeft, canvasSize._data[ 0U ], units ),
        ResolvePixelLength ( _css._marginTop, canvasSize._data[ 1U ], units )
    );

    GXVec2 const paddingTopLeft ( ResolvePixelLength ( _css._paddingLeft, canvasSize._data[ 0U ], units ),
        ResolvePixelLength ( _css._paddingTop, canvasSize._data[ 1U ], units )
    );

    GXVec2 alpha {};
    alpha.Sum ( marginTopLeft, paddingTopLeft );

    GXVec2 penLocation {};

    switch ( _css._position )
    {
        case PositionProperty::eValue::Absolute:
            penLocation.Sum ( info._parentTopLeft, alpha );
        break;

        case PositionProperty::eValue::Static:
            if ( _css._display == DisplayProperty::eValue::InlineBlock )
            {
                penLocation.Sum ( info._penLocation, alpha );
            }
            else
            {
                // 'block' territory. Element should be started from new line.
                GXVec2 beta {};
                beta.Sum ( alpha, GXVec2 ( info._parentTopLeft._data[ 0U ], info._currentLineHeight ) );

                penLocation._data[ 0U ] = beta._data[ 0U ];
                penLocation._data[ 1U ] += beta._data[ 1U ];
            }
        break;

        default:
            AV_ASSERT ( false )
        return;
    }

    ApplyLayoutInfo childInfo
    {
        ._canvasSize = GXVec2 ( ResolvePixelLength ( _css._width, canvasSize._data[ 0U ], units ),
            ResolvePixelLength ( _css._height, canvasSize._data[ 1U ], units )
        ),

        ._cssUnits = info._cssUnits,
        ._currentLineHeight = 0.0F,
        ._fontStorage = info._fontStorage,
        ._newLineHeight = 0.0F,
        ._newLines = 0U,
        ._parentTopLeft = penLocation,
        ._penLocation = penLocation,
        ._renderer = info._renderer
    };

    _lineHeights.clear ();
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

    if ( _css._position == PositionProperty::eValue::Absolute )
        return;

    GXVec2 const marginBottomRight ( ResolvePixelLength ( _css._marginRight, canvasSize._data[ 0U ], units ),
        ResolvePixelLength ( _css._marginBottom, canvasSize._data[ 1U ], units )
    );

    GXVec2 const paddingBottomRight ( ResolvePixelLength ( _css._paddingRight, canvasSize._data[ 0U ], units ),
        ResolvePixelLength ( _css._paddingBottom, canvasSize._data[ 1U ], units )
    );

    GXVec2 canvas = childInfo._canvasSize;

    if ( _css._height.GetType () == LengthValue::eType::Auto )
        canvas._data[ 1U ] = childInfo._penLocation._data[ 1U ] - info._penLocation._data[ 1U ] + _lineHeights.back ();

    GXVec2 gamma {};
    gamma.Sum ( canvas, alpha );

    GXVec2 yotta {};
    yotta.Sum ( marginBottomRight, paddingBottomRight );

    GXVec2 blockSize {};
    blockSize.Sum ( gamma, yotta );

    if ( _css._display == DisplayProperty::eValue::Block )
    {
        // Block starts from new line and consumes whole parent block line.
        info._newLines = 1U;
        info._penLocation._data[ 0U ] = info._parentTopLeft._data[ 0U ];
        info._penLocation._data[ 1U ] += info._currentLineHeight + blockSize._data[ 1U ];
        return;
    }

    // 'inline-block' territory.

    bool const firstBlock = info._penLocation.IsEqual ( info._parentTopLeft );

    float const rest = info._canvasSize._data[ 0U ] + info._parentTopLeft._data[ 0U ] - info._penLocation._data[ 0U ];
    bool const blockCanFit = rest >= blockSize._data[ 0U ];

    if ( firstBlock | blockCanFit )
    {
        info._currentLineHeight = std::max ( info._currentLineHeight, blockSize._data[ 1U ] );
        info._newLines = 0U;
        info._penLocation._data[ 0U ] += blockSize._data[ 0U ];
        return;
    }

    // Block goes to the new line of parent block.
    info._newLines = 1U;
    info._newLineHeight = blockSize._data[ 1U ];
    info._penLocation._data[ 0U ] = blockSize._data[ 0U ];
    info._penLocation._data[ 1U ] += info._currentLineHeight;
}

void DIVUIElement::Render () noexcept
{
    // TODO

    for ( auto* child : _children )
    {
        child->Render ();
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

float DIVUIElement::ResolvePixelLength ( LengthValue const &length,
    float parentLength,
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
        return 1.0e-2F * parentLength * length.GetValue ();

        case LengthValue::eType::Auto:
        return 1.0F;

        default:
            AV_ASSERT ( false )
        return 0.0F;
    }

}

} // namespace pbr
