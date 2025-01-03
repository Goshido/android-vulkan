#include <precompiled_headers.hpp>
#include <GXCommon/GXMath.hpp>
#include <logger.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <theme.hpp>
#include <ui_dialog_box.hpp>


namespace editor {

namespace {

constexpr float RESIZE_THICKNESS = 2.0F;
constexpr float DRAG_THICKNESS = 3.5F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UIDialogBox::Gizmo::Gizmo ( eCursor cursor ) noexcept:
    _cursor ( cursor )
{
    // NOTHING
}

bool UIDialogBox::Gizmo::OnMouseMove ( MessageQueue& messageQueue, MouseMoveEvent const& event ) noexcept
{
    if ( !_rect.IsOverlapped ( event._x, event._y ) )
        return false;

    if ( event._eventID - _eventID > 1U )
    {
        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::ChangeCursor,
                ._params = reinterpret_cast<void*> ( _cursor ),
                ._serialNumber = 0U
            }
        );
    }

    _eventID = event._eventID;
    return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UIDialogBox::SetRect ( Rect const &rect ) noexcept
{
    _isChanged = true;
    _rect = rect;
    UpdateAreas ();
}

void UIDialogBox::SetMinSize ( pbr::LengthValue const &width, pbr::LengthValue const &height ) noexcept
{
    _minWidthCSS = width;
    _minHeightCSS = height;
    UpdateMinSize ();
}

UIDialogBox::UIDialogBox ( MessageQueue &messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    pbr::CSSComputedValues &css = _div.GetCSS ();
    css._position = pbr::PositionProperty::eValue::Absolute;
    css._backgroundColor = theme::BACKGROUND_COLOR;
    css._backgroundSize = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );

    css._marginBottom = theme::ZERO_LENGTH;
    css._marginLeft = theme::ZERO_LENGTH;
    css._marginRight = theme::ZERO_LENGTH;
    css._marginTop = theme::ZERO_LENGTH;
    css._paddingBottom = theme::ZERO_LENGTH;
    css._paddingLeft = theme::ZERO_LENGTH;
    css._paddingRight = theme::ZERO_LENGTH;
    css._paddingTop = theme::ZERO_LENGTH;
    css._right = pbr::LengthValue ( pbr::LengthValue::eType::Auto, 0.0F );
    css._bottom = pbr::LengthValue ( pbr::LengthValue::eType::Auto, 0.0F );
}

void UIDialogBox::OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept
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
        _initialX = x;
        _initialY = y;

        _leftMask = left;
        _topMask = top;
        _rightMask = right;
        _bottomMask = bottom;

        _safeDX = _rect.GetWidth () - _minWidth;
        _safeDY = _rect.GetHeight () - _minHeight;

        _messageQueue.EnqueueBack (
            {
                ._type = eMessageType::StartWidgetCaptureMouse,
                ._params = this,
                ._serialNumber = 0U
            }
        );
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

void UIDialogBox::OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept
{
    if ( !_dragState | ( event._key != eKey::LeftMouseButton ) ) [[likely]]
        return;

    _dragState = false;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StopWidgetCaptureMouse,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void UIDialogBox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    if ( _dragState ) [[unlikely]]
    {
        DoDrag ( event );
        return;
    }

    DoHover ( event );
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
        ._pen = GXVec2 ( 0.0F, 0.0F ),
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

void UIDialogBox::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    _div.Submit ( info );
}

bool UIDialogBox::UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept
{
    pbr::UIElement::UpdateInfo info
    {
        ._fontStorage = &fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._parentTopLeft = GXVec2 ( 0.0F, 0.0F ),
        ._pen = GXVec2 ( 0.0F, 0.0F )
    };

    return _div.UpdateCache ( info );
}

void UIDialogBox::DoDrag ( MouseMoveEvent const &event ) noexcept
{
    int32_t const deltaX = event._x - _initialX;
    int32_t const deltaY = event._y - _initialY;

    // Step 1. Blindly applying delta size...

    auto dx = static_cast<uint32_t> ( deltaX );
    auto dy = static_cast<uint32_t> ( deltaY );

    Rect const newRect (
        _initialRect._left + static_cast<int32_t> ( dx & _leftMask ),
        _initialRect._right + static_cast<int32_t> ( dx & _rightMask ),
        _initialRect._top + static_cast<int32_t> ( dy & _topMask ),
        _initialRect._bottom + static_cast<int32_t> ( dy & _bottomMask )
    );

    // Step 2. Checking safe boundaries with respect of minimal size...

    int32_t const width = newRect.GetWidth ();
    int32_t const height = newRect.GetHeight ();

    // Taking into account input delta size sign...
    int32_t const safeDXCases[] = { -_safeDX, _safeDX };
    int32_t const safeDYCases[] = { -_safeDY, _safeDY };

    uint32_t const dXCases[] = { dx, static_cast<uint32_t> ( safeDXCases[ static_cast<size_t> ( deltaX > 0 ) ] ) };
    uint32_t const dYCases[] = { dy, static_cast<uint32_t> ( safeDYCases[ static_cast<size_t> ( deltaY > 0 ) ] ) };

    dx = dXCases[ static_cast<size_t> ( width < _minWidth ) ];
    dy = dYCases[ static_cast<size_t> ( height < _minHeight ) ];

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
    MessageQueue &queue = _messageQueue;

    bool const handled = _dragArea.OnMouseMove ( queue, event ) ||
        _resizeUp.OnMouseMove ( queue, event ) ||
        _resizeDown.OnMouseMove ( queue, event ) ||
        _resizeLeft.OnMouseMove ( queue, event ) ||
        _resizeRight.OnMouseMove ( queue, event ) ||
        _resizeTopLeft.OnMouseMove ( queue, event ) ||
        _resizeTopRight.OnMouseMove ( queue, event ) ||
        _resizeBottomLeft.OnMouseMove ( queue, event ) ||
        _resizeBottomRight.OnMouseMove ( queue, event );

    if ( handled )
        return;

    size_t const eventID = event._eventID;

    if ( eventID - _eventID > 1U )
    {
        queue.EnqueueBack (
            {
                ._type = eMessageType::ChangeCursor,
                ._params = reinterpret_cast<void*> ( eCursor::Arrow ),
                ._serialNumber = 0U
            }
        );
    }

    _eventID = eventID;
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

    auto const apply = [ &units ] ( int32_t &dst, pbr::LengthValue const &src ) noexcept {
        switch ( src.GetType () )
        {
            case pbr::LengthValue::eType::MM:
                dst = static_cast<int32_t> ( units._fromMM * src.GetValue () );
            break;

            case pbr::LengthValue::eType::PT:
                dst = static_cast<int32_t> ( units._fromPT * src.GetValue () );
            break;

            case pbr::LengthValue::eType::PX:
                dst = static_cast<int32_t> ( units._fromPX * src.GetValue () );
            break;

            case pbr::LengthValue::eType::EM:
                [[fallthrough]];
            case pbr::LengthValue::eType::Auto:
                [[fallthrough]];
            case pbr::LengthValue::eType::Percent:
                [[fallthrough]];
            default:
                android_vulkan::LogWarning ( "UIDialogBox::UpdateMinSize - Only MM, PT and PX units are supported. "
                    "Skipping." );
            break;
        }
    };

    apply ( _minWidth, _minWidthCSS );
    apply ( _minHeight, _minHeightCSS );
}

} // namespace editor
