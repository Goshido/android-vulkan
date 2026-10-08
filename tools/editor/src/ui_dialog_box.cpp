#include <precompiled_headers.hpp>
#include <GXCommon/GXMath.hpp>
#include <logger.hpp>
#include <message_queue.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <ui_dialog_box.hpp>


namespace editor {

namespace {

constexpr float RESIZE_THICKNESS = 2.0F;
constexpr float DRAG_THICKNESS = 5.5F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UIDialogBox::Gizmo::Gizmo ( eCursor cursor ) noexcept:
    _cursor ( cursor )
{
    // NOTHING
}

bool UIDialogBox::Gizmo::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    if ( !_rect.IsOverlapped ( event._x, event._y ) )
        return false;

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return true;

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::ChangeCursor,
            [ cursor = std::bit_cast<void*> ( _cursor ) ] () noexcept {
                return cursor;
            }
        )
    );

    return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UIDialogBox::SetRect ( Rect const &rect ) noexcept
{
    _isChanged = true;
    _rect = rect;

    VkOffset2D const size = rect.GetSize ();
    ApplyMinSizeConstraints ( size );
    ApplyMaxSizeConstraints ( size );
    UpdateAreas ();
}

void UIDialogBox::SetMinSize ( pbr::LengthValue const &width, pbr::LengthValue const &height ) noexcept
{
    _isChanged = true;
    _minWidthCSS = width;
    _minHeightCSS = height;
    UpdateMinSize ();
}

void UIDialogBox::SetMaxSize ( pbr::LengthValue const &width, pbr::LengthValue const &height ) noexcept
{
    _isChanged = true;
    _maxWidthCSS = width;
    _maxHeightCSS = height;
    UpdateMaxSize ();
}

UIDialogBox::UIDialogBox ( std::string &&name ) noexcept:
    _div (
        {
            ._backgroundColor = theme::BACKGROUND_COLOR,
            ._backgroundSize = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TEXT_COLOR_NORMAL,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::NORMAL_LINE_HEIGHT,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Absolute,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = theme::SMALL_BUTTON_HEIGHT,
            ._height = theme::SMALL_BUTTON_HEIGHT
        },

        std::move ( name )
    )
{
    UpdateMaxSize ();
}

void UIDialogBox::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton )
        return;

    int32_t const x = event._x;
    int32_t const y = event._y;
    constexpr uint32_t active = std::numeric_limits<uint32_t>::max ();
    constexpr uint32_t passive = 0U;

    auto const startDrag = [ this, x, y ] ( uint32_t left, uint32_t top, uint32_t right, uint32_t bottom ) noexcept {
        _dragState = true;
        _initialRect = _rect;

        _initialMouse =
        {
            .x = x,
            .y = y
        };

        _leftMask = left;
        _topMask = top;
        _rightMask = right;
        _bottomMask = bottom;

        _safeDelta =
        {
            .x = _rect.GetWidth () - _minSize.x,
            .y = _rect.GetHeight () - _minSize.y
        };

        CaptureMouse ();
    };

    if ( _dragArea._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( active, active, active, active );
        return;
    }

    if ( _resizeUp._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( passive, active, passive, passive );
        return;
    }

    if ( _resizeDown._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( passive, passive, passive, active );
        return;
    }

    if ( _resizeLeft._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( active, passive, passive, passive );
        return;
    }

    if ( _resizeRight._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( passive, passive, active, passive );
        return;
    }

    if ( _resizeTopLeft._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( active, active, passive, passive );
        return;
    }

    if ( _resizeTopRight._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( passive, active, active, passive );
        return;
    }

    if ( _resizeBottomLeft._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( active, passive, passive, active );
        return;
    }

    if ( _resizeBottomRight._rect.IsOverlapped ( x, y ) )
    {
        startDrag ( passive, passive, active, active );
    }
}

void UIDialogBox::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( _dragState & ( event._key == eKey::LeftMouseButton ) ) [[likely]]
    {
        _dragState = false;
        ReleaseMouse ();
    }
}

void UIDialogBox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    if ( _dragState ) [[unlikely]]
    {
        DoDrag ( event );
        return;
    }

    DoHover ( event );
    Widget::OnMouseMove ( event );
}

void UIDialogBox::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    _div.Submit ( info );
}

Widget::LayoutStatus UIDialogBox::ApplyLayout ( android_vulkan::Renderer &renderer,
    pbr::FontStorage &fontStorage
) noexcept
{
    VkExtent2D const viewport = renderer.GetViewportResolution ();

    _lineHeights.clear ();
    _lineHeights.push_back ( 0.0F );

    pbr::UIElement::ApplyInfo info
    {
        ._canvasSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._fontStorage = &fontStorage,
        ._hasChanges = _isChanged,
        ._lineHeights = &_lineHeights,
        ._parentPaddingExtent = GXVec2::ZERO,
        ._pen = GXVec2::ZERO,
        ._renderer = &renderer,
        ._vertices = 0U
    };

    _div.ApplyLayout ( info );
    _isChanged = false;

    return
    {
        ._hasChanges = info._hasChanges,
        ._neededUIVertices = info._vertices
    };
}

bool UIDialogBox::UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept
{
    pbr::UIElement::UpdateInfo info
    {
        ._fontStorage = &fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._parentTopLeft = GXVec2::ZERO,
        ._pen = GXVec2::ZERO
    };

    return _div.UpdateCache ( info );
}

void UIDialogBox::ApplyMinSizeConstraints ( VkOffset2D const &size ) noexcept
{
    int32_t const dW[] = { 0, _minSize.x - size.x };
    int32_t const dH[] = { 0, _minSize.y - size.y };

    _rect._right += dW[ static_cast<uint32_t> ( size.x < _minSize.x ) ];
    _rect._bottom += dH[ static_cast<uint32_t> ( size.y < _minSize.y ) ];
}

void UIDialogBox::ApplyMaxSizeConstraints ( VkOffset2D const &size ) noexcept
{
    int32_t const dW[] = { 0, _maxSize.x - size.x };
    int32_t const dH[] = { 0, _maxSize.y - size.y };

    _rect._right += dW[ static_cast<uint32_t> ( size.x > _maxSize.x ) ];
    _rect._bottom += dH[ static_cast<uint32_t> ( size.y > _maxSize.y ) ];
}

void UIDialogBox::DoDrag ( MouseMoveEvent const &event ) noexcept
{
    VkOffset2D const delta
    {
        .x = event._x - _initialMouse.x,
        .y = event._y - _initialMouse.y
    };

    // Step 1. Blindly applying delta size...
    auto dx = static_cast<uint32_t> ( delta.x );
    auto dy = static_cast<uint32_t> ( delta.y );

    Rect const newRect (
        _initialRect._left + static_cast<int32_t> ( dx & _leftMask ),
        _initialRect._right + static_cast<int32_t> ( dx & _rightMask ),
        _initialRect._top + static_cast<int32_t> ( dy & _topMask ),
        _initialRect._bottom + static_cast<int32_t> ( dy & _bottomMask )
    );

    // Step 2. Checking safe boundaries with respect of min/max size...
    VkOffset2D const size = newRect.GetSize ();

    // Taking into account input delta size sign...
    int32_t const safeDXCases[] = { -_safeDelta.x, _safeDelta.x };
    int32_t const safeDYCases[] = { -_safeDelta.y, _safeDelta.y };

    uint32_t const dXCases[] = { dx, static_cast<uint32_t> ( safeDXCases[ static_cast<uint32_t> ( delta.x > 0 ) ] ) };
    uint32_t const dYCases[] = { dy, static_cast<uint32_t> ( safeDYCases[ static_cast<uint32_t> ( delta.y > 0 ) ] ) };

    dx = dXCases[ static_cast<uint32_t> ( ( size.x < _minSize.x ) | ( size.x > _maxSize.x ) ) ];
    dy = dYCases[ static_cast<uint32_t> ( ( size.y < _minSize.y ) | ( size.y > _maxSize.y ) ) ];

    SetRect (
        Rect (
            _initialRect._left + static_cast<int32_t> ( dx & _leftMask ),
            _initialRect._right + static_cast<int32_t> ( dx & _rightMask ),
            _initialRect._top + static_cast<int32_t> ( dy & _topMask ),
            _initialRect._bottom + static_cast<int32_t> ( dy & _bottomMask )
        )
    );
}

void UIDialogBox::DoHover ( MouseMoveEvent const &event ) noexcept
{
    bool const handled = _dragArea.OnMouseMove ( event ) ||
        _resizeUp.OnMouseMove ( event ) ||
        _resizeDown.OnMouseMove ( event ) ||
        _resizeLeft.OnMouseMove ( event ) ||
        _resizeRight.OnMouseMove ( event ) ||
        _resizeTopLeft.OnMouseMove ( event ) ||
        _resizeTopRight.OnMouseMove ( event ) ||
        _resizeBottomLeft.OnMouseMove ( event ) ||
        _resizeBottomRight.OnMouseMove ( event );

    if ( handled )
        return;

    if ( size_t const eventID = event._eventID; eventID - std::exchange ( _eventID, eventID ) > 1U ) [[unlikely]]
    {
        ChangeCursor ( eCursor::Arrow );
    }
}

void UIDialogBox::UpdateAreas () noexcept
{
    GXVec2 const a = _rect.GetTopLeft ();
    GXVec2 const b = _rect.GetBottomRight ();

    float const fromMM = pbr::CSSUnitToDevicePixel::GetInstance ()._fromMM;
    float const resizeThickness = RESIZE_THICKNESS * fromMM;
    GXVec2 const t ( resizeThickness, resizeThickness );

    GXVec2 c {};
    c.Sum ( a, t );

    GXVec2 d {};
    d.Subtract ( b, t );

    _dragArea._rect.From ( c, GXVec2 ( d._data[ 0U ], c._data[ 1U ] + DRAG_THICKNESS * fromMM ) );

    _resizeUp._rect.From ( GXVec2 ( c._data[ 0U ], a._data[ 1U ] ), GXVec2 ( d._data[ 0U ], c._data[ 1U ] ) );
    _resizeDown._rect.From ( GXVec2 ( c._data[ 0U ], d._data[ 1U ] ), GXVec2 ( d._data[ 0U ], b._data[ 1U ] ) );
    _resizeLeft._rect.From ( GXVec2 ( a._data[ 0U ], c._data[ 1U ] ), GXVec2 ( c._data[ 0U ], d._data[ 1U ] ) );
    _resizeRight._rect.From ( GXVec2 ( d._data[ 0U ], c._data[ 1U ] ), GXVec2 ( b._data[ 0U ], d._data[ 1U ] ) );

    _resizeTopLeft._rect.From ( a, c );
    _resizeTopRight._rect.From ( GXVec2 ( d._data[ 0U ], a._data[ 1U ] ), GXVec2 ( b._data[ 0U ], c._data[ 1U ] ) );
    _resizeBottomLeft._rect.From ( GXVec2 ( a._data[ 0U ], d._data[ 1U ] ), GXVec2 ( c._data[ 0U ], b._data[ 1U ] ) );
    _resizeBottomRight._rect.From ( d, b );

    _rect.ToCSSBounds ( _div.GetCSS () );
}

void UIDialogBox::UpdateMinSize () noexcept
{
    pbr::CSSUnitToDevicePixel const &units = pbr::CSSUnitToDevicePixel::GetInstance ();

    _minSize =
    {
        .x = ResolveLength ( _minWidthCSS, _minSize.x, units ),
        .y = ResolveLength ( _minHeightCSS, _minSize.y, units )
    };

    ApplyMinSizeConstraints ( _rect.GetSize () );
    UpdateAreas ();
}

void UIDialogBox::UpdateMaxSize () noexcept
{
    pbr::CSSUnitToDevicePixel const &units = pbr::CSSUnitToDevicePixel::GetInstance ();

    _maxSize =
    {
        .x = ResolveLength ( _maxWidthCSS, _maxSize.x, units ),
        .y = ResolveLength ( _maxHeightCSS, _maxSize.y, units )
    };

    ApplyMaxSizeConstraints ( _rect.GetSize () );
    UpdateAreas ();
}

int32_t UIDialogBox::ResolveLength ( pbr::LengthValue const &value,
    int32_t defaultValue,
    pbr::CSSUnitToDevicePixel const &units
) noexcept
{
    switch ( value.GetType () )
    {
        case pbr::LengthValue::eType::MM:
        return static_cast<int32_t> ( units._fromMM * value.GetValue () );

        case pbr::LengthValue::eType::PT:
        return static_cast<int32_t> ( units._fromPT * value.GetValue () );

        case pbr::LengthValue::eType::PX:
        return static_cast<int32_t> ( units._fromPX * value.GetValue () );

        case pbr::LengthValue::eType::Auto:
            [[fallthrough]];
        case pbr::LengthValue::eType::EM:
            [[fallthrough]];
        case pbr::LengthValue::eType::Inherit:
            [[fallthrough]];
        case pbr::LengthValue::eType::Percent:
            [[fallthrough]];
        case pbr::LengthValue::eType::Unitless:
            [[fallthrough]];
        default:
            android_vulkan::LogWarning ( "UIDialogBox::ResolveLength - Only MM, PT and PX units are supported. "
                "Skipping." );
        break;
    }

    return defaultValue;
}

} // namespace editor
