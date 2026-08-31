#include <precompiled_headers.hpp>
#include <theme.hpp>
#include <viewport_widget.hpp>
#include <workspace.hpp>


namespace editor {

namespace {

constexpr float FREE_FLY_ORIENTATION_SPEED = 4.0e-3F;
constexpr float FREE_FLY_MOVE_SPEED = 4.0F;
constexpr float FREE_FLY_SPRINT_SPEED = 10.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+3F;
constexpr float FOV_Y = GXDegToRad ( 60.0F );
constexpr float MOVE_SPEED_THRESHOLD = 1.0e-4F;

} // namespace

ViewportWidget::ViewportWidget () noexcept:
    _div (
        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        "viewport"
    ),

    _selectionBody ( _div,

        {
            ._backgroundColor = theme::YELLOW_PAINT_TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::HEADER_HEIGHT,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::HEADER_HEIGHT,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 400.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 200.0F )
        },

        "Selection (body)"
    ),

    _selectionTop ( _selectionBody,

        {
            ._backgroundColor = theme::YELLOW_PAINT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F )
        },

        "Selection (top)"
    ),

    _selectionRight ( _selectionBody,

        {
            ._backgroundColor = theme::YELLOW_PAINT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        "Selection (right)"
    ),

    _selectionBottom ( _selectionBody,

        {
            ._backgroundColor = theme::YELLOW_PAINT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::AUTO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F )
        },

        "Selection (bottom)"
    ),

    _selectionLeft ( _selectionBody,

        {
            ._backgroundColor = theme::YELLOW_PAINT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        "Selection (left)"
    )
{
    _selectionBody.AppendChildElement ( _selectionTop );
    _selectionBody.AppendChildElement ( _selectionRight );
    _selectionBody.AppendChildElement ( _selectionBottom );
    _selectionBody.AppendChildElement ( _selectionLeft );

    _div.AppendChildElement ( _selectionBody );
    _selectionBody.Hide ();

    //_rotateTool.Activate ();
    _moveTool.Activate ();
    //_scaleTool.Activate ();
}

void ViewportWidget::Init () noexcept
{
    _useMoveTool = Hotkey ( eKey::KeyW,
        false,
        false,
        false,

        [ this ] () noexcept {
            SwitchTool ( _moveTool );
        }
    );

    _useRotateTool = Hotkey ( eKey::KeyE,
        false,
        false,
        false,

        [ this ] () noexcept {
            SwitchTool ( _rotateTool );
        }
    );

    _useSelectTool = Hotkey ( eKey::KeyQ,
        false,
        false,
        false,

        [ this ] () noexcept {
            SwitchTool ( _selectTool );
        }
    );

    _useScaleTool = Hotkey ( eKey::KeyR,
        false,
        false,
        false,

        [ this ] () noexcept {
            SwitchTool ( _scaleTool );
        }
    );
}

void ViewportWidget::Destroy () noexcept
{
    _useSelectTool = {};
    _useMoveTool = {};
    _useRotateTool = {};
    _useScaleTool = {};
}

void ViewportWidget::Update ( float deltaTime, float dpi ) noexcept
{
    eNavigationMode const old = _navigationMode;

    auto const captureInput = [ this, old ] () noexcept {
        if ( old == eNavigationMode::None ) [[unlikely]]
        {
            CaptureMouse ();
            SetFocus ();
        }
    };

    ResolveNavigationMode ();

    switch ( _navigationMode )
    {
        case eNavigationMode::FreeFly:
            captureInput ();
            DoFreeFly ( deltaTime, dpi );
        return;

        case eNavigationMode::Orbit:
            captureInput ();
            DoOrbit ();
        return;

        case eNavigationMode::None:
            [[fallthrough]];
        default:
            // IMPOSSIBLE
        break;
    }

    if ( old == eNavigationMode::None ) [[likely]]
        return;

    ReleaseMouse ();
    KillFocus ();
    _state = {};
}

GXMat4 const &ViewportWidget::GetProjection () const noexcept
{
    return _projection;
}

GXQuat const &ViewportWidget::GetOrientation () const noexcept
{
    return _orientation;
}

GXVec3 const &ViewportWidget::GetLocation () const noexcept
{
    return _location;
}

GXVec3 ViewportWidget::GetVI () const noexcept
{
    float const t = std::tan ( 0.5F * FOV_Y );
    GXVec3 forward {};
    _orientation.GetForward ( forward );

    GXVec3 result {};
    result.Multiply ( forward, ( t + t ) * _invHeight );
    return result;
}

void ViewportWidget::OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept
{
    UpdateKeyboardState ( key, modifier, 1U );

    if ( !_selectionMode )
        return;

    // [2026/07/21] Windows 11 Pro 25H2 26200.8894: OS triggers this handler many times when
    // any keyboard key is pressed. The behaviour is similar to autorepeat using long press during typing.
    // It's needed to filter such repeat events.
    Selection::eMode const old = *_selectionMode;
    UpdateSelectionMode ();

    if ( *_selectionMode != old )
    {
        std::ignore = Workspace::Instance ().GetSelection ().Update ( _mouseNow, *_selectionMode );
    }
}

void ViewportWidget::OnKeyboardKeyUp ( eKey key, KeyModifier modifier ) noexcept
{
    UpdateKeyboardState ( key, modifier, 0U );

    if ( _selectionMode )
    {
        UpdateSelectionMode ();
        std::ignore = Workspace::Instance ().GetSelection ().Update ( _mouseNow, *_selectionMode );
    }
}

void ViewportWidget::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    UpdateMouseState ( event, 1U );

    if ( event._key != eKey::LeftMouseButton )
        return;

    constexpr Selection::eMode const cases[] = { Selection::eMode::New, Selection::eMode::Toggle };
    _selectionMode = std::optional<Selection::eMode> ( cases[ static_cast<size_t> ( _state._ctrl | _state._shift ) ] );
    Workspace::Instance ().GetSelection ().Begin ( _mouseNow, *_selectionMode );

    _selectionBody.Show ();
    UpdateSelection ( _mouseNow.x, _mouseNow.y, 0, 0 );
    CaptureMouse ();
    SetFocus ();
}

void ViewportWidget::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    UpdateMouseState ( event, 0U );

    if ( _selectionMode )
    {
        UpdateSelectionMode ();

        Workspace::Instance ().GetSelection ().End (
            VkOffset2D
            {
                .x = event._x,
                .y = event._y
            },

            *_selectionMode
        );

        _selectionMode = std::nullopt;
    }

    if ( event._key != eKey::LeftMouseButton )
        return;

    ReleaseMouse ();
    KillFocus ();
    _selectionBody.Hide ();
    _selectionDrag = false;
}

void ViewportWidget::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) > 1U ) [[unlikely]]
        ChangeCursor ( eCursor::Arrow );

    _mouseNow =
    {
        .x = event._x,
        .y = event._y
    };

    // FUCK
    GXMat3 basis {};
    basis.FromFast ( _orientation );

    _moveTool.Update ( ComputeRayDirection ( basis ), _location, GetVI (), _state._leftMouseButton == 1U );

    //_mouseNow =
    //{
    //    .x = 835,
    //    .y = 274
    //};

    //_moveTool.Update ( ComputeRayDirection ( basis ), _location, GetVI (), true );

    //_mouseNow =
    //{
    //    .x = 750,
    //    .y = 350
    //};

    //_moveTool.Update ( ComputeRayDirection ( basis ), _location, GetVI (), true );

    if ( !_selectionMode )
        return;

    // [2026/07/21] Selection rectangle frame pacing degrades because keyboard input accelerates OS mouse-move
    // events. To prevent CPU spin-locking, the OS message pump utilizes minimal sleep intervals. The combination
    // of these two factors causes actual mouse coordinates to arrive at irregular intervals.
    auto const rect = Workspace::Instance ().GetSelection ().Update (
        VkOffset2D
        {
            .x = event._x,
            .y = event._y
        },

        *_selectionMode
    );

    UpdateSelection ( rect->_left, rect->_top, rect->GetWidth (), rect->GetHeight () );
    _selectionDrag = true;
}

Widget::LayoutStatus ViewportWidget::ApplyLayout ( android_vulkan::Renderer &renderer,
    pbr::FontStorage &fontStorage
) noexcept
{
    VkExtent2D const &viewport = renderer.GetViewportResolution ();

    GXVec2 const size ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) );
    _invHeight = 1.0F / size._data[ 1U ];
    _projection.Perspective ( FOV_Y, size._data[ 0U ] * _invHeight, Z_NEAR, Z_FAR );

    _lineHeights.clear ();
    _lineHeights.push_back ( 0.0F );

    pbr::UIElement::ApplyInfo applyInfo
    {
        ._canvasSize = size,
        ._fontStorage = &fontStorage,
        ._hasChanges = false,
        ._lineHeights = &_lineHeights,
        ._parentPaddingExtent = GXVec2::ZERO,
        ._pen = GXVec2::ZERO,
        ._renderer = &renderer,
        ._vertices = 0U
    };

    _div.ApplyLayout ( applyInfo );

    return
    {
        ._hasChanges = applyInfo._hasChanges,
        ._neededUIVertices = applyInfo._vertices
    };
}

void ViewportWidget::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    _div.Submit ( info );
    _rect.From ( _div.GetAbsoluteRect () );
}

bool ViewportWidget::UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept
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

GXVec3 ViewportWidget::ComputeRayDirection ( GXMat3 const &basis ) const noexcept
{
    // See <repo>/docs/gizmo-rendering.md#pixel-coverage
    float const t = std::tan ( 0.5F * FOV_Y );
    GXVec3 vi {};
    GXVec3 const &forward = basis.Forward ();
    vi.Multiply ( forward, ( t + t ) * _invHeight );

    GXVec2 alpha ( _div.GetAbsoluteRect ()._bottomRight );
    alpha.Multiply ( alpha, GXVec2 ( -0.5F, 0.5F ) );
    GXVec2 beta ( static_cast<float> ( _mouseNow.x ), static_cast<float> ( -_mouseNow.y ) );
    alpha.Sum ( alpha, beta );
    alpha.Sum ( alpha, GXVec2 ( 0.5F, -0.5F ) );
    alpha.Multiply ( alpha, vi.DotProduct ( forward ) );

    GXVec3 result {};
    basis.MultiplyVectorMatrix ( result, GXVec3 ( alpha._data[ 0U ], alpha._data[ 1U ], 1.0F ) );
    result.Normalize ();
    return result;
}

void ViewportWidget::UpdateKeyboardState ( eKey key, KeyModifier modifier, uint8_t matchValue ) noexcept
{
    GX_DISABLE_WARNING ( 4061 )

    switch ( key )
    {
        case eKey::KeyW:
            _state._forward = matchValue;
        break;

        case eKey::KeyA:
            _state._left = matchValue;
        break;

        case eKey::KeyS:
            _state._backward = matchValue;
        break;

        case eKey::KeyD:
            _state._right = matchValue;
        break;

        default:
            // NOTHING
        break;
    }

    GX_ENABLE_WARNING ( 4061 )

    _state._ctrl = static_cast<uint8_t> ( modifier.AnyCtrlPressed () );
    _state._shift = static_cast<uint8_t> ( modifier.AnyShiftPressed () );
    _state._alt = static_cast<uint8_t> ( modifier.AnyAltPressed () );
}

void ViewportWidget::UpdateMouseState ( MouseButtonEvent const &event, uint8_t matchValue ) noexcept
{
    GX_DISABLE_WARNING ( 4061 )

    switch ( event._key )
    {
        case eKey::LeftMouseButton:
            _state._leftMouseButton = matchValue;
        break;

        case eKey::MiddleMouseButton:
            _state._middleMouseButton = matchValue;
        break;

        default:
            // NOTHING
        break;
    }

    GX_ENABLE_WARNING ( 4061 )

    KeyModifier const &modifier = event._modifier;
    _state._ctrl = static_cast<uint8_t> ( modifier.AnyCtrlPressed () );
    _state._shift = static_cast<uint8_t> ( modifier.AnyShiftPressed () );
    _state._alt = static_cast<uint8_t> ( modifier.AnyAltPressed () );

    _mouseNow =
    {
        .x = event._x,
        .y = event._y
    };
}

void ViewportWidget::UpdateSelection ( int32_t left, int32_t top, int32_t width, int32_t height ) noexcept
{
    float const scale = pbr::CSSUnitToDevicePixel::GetInstance ()._devicePXtoCSSPX;

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::InvokeRenderSession,
            [
                this,
                left = pbr::LengthValue ( pbr::LengthValue::eType::PX, scale * static_cast<float> ( left ) ),
                top = pbr::LengthValue ( pbr::LengthValue::eType::PX, scale * static_cast<float> ( top ) ),
                width = pbr::LengthValue ( pbr::LengthValue::eType::PX, scale * static_cast<float> ( width ) ),
                height = pbr::LengthValue ( pbr::LengthValue::eType::PX, scale * static_cast<float> ( height ) )
            ] () noexcept -> void* {
                pbr::CSSComputedValues &css = _selectionBody.GetCSS ();
                css._left = left;
                css._top = top;
                css._width = width;
                css._height = height;

                // NOLINTNEXTLINE - downcast
                static_cast<pbr::DIVUIElement &> ( _selectionBody.GetNativeElement () ).Update ();
                return nullptr;
            }
        )
    );
}

void ViewportWidget::UpdateSelectionMode () noexcept
{
    constexpr Selection::eMode const cases[] =
    {
        Selection::eMode::New,
        Selection::eMode::Toggle,
        Selection::eMode::Toggle,
        Selection::eMode::Toggle,
        Selection::eMode::New,
        Selection::eMode::Add,
        Selection::eMode::Remove,
        Selection::eMode::Add
    };

    *_selectionMode = cases[
        static_cast<size_t> (
            _state._shift |
            ( _state._ctrl << 1U ) |
            ( static_cast<uint8_t> ( _selectionDrag ) << 2U )
        )
    ];
}

void ViewportWidget::ResolveNavigationMode () noexcept
{
    constexpr eNavigationMode const cases[] =
    {
        eNavigationMode::None,
        eNavigationMode::FreeFly,
        eNavigationMode::Orbit,
        eNavigationMode::Orbit
    };

    auto const selector = static_cast<size_t> (
        _state._middleMouseButton | ( ( _state._leftMouseButton & _state._alt ) << 1U )
    );

    eNavigationMode const current = cases[ selector ];
    eNavigationMode const resultCases[] = { _navigationMode, current };

    auto const resultSelector =
        static_cast<size_t> ( ( _navigationMode == eNavigationMode::None ) | ( current == eNavigationMode::None ) );

    eNavigationMode const old = std::exchange ( _navigationMode, resultCases[ resultSelector ] );

    VkOffset2D const mouseCases[] = { _mouseCommit, _mouseNow };
    _mouseCommit = mouseCases[ static_cast<size_t> ( old != _navigationMode ) ];
}

void ViewportWidget::DoFreeFly ( float deltaTime, float dpi ) noexcept
{
    _eulerAngles.Sum ( _eulerAngles,
        FREE_FLY_ORIENTATION_SPEED / dpi,

        GXVec2 ( static_cast<float> ( _mouseNow.y - _mouseCommit.y ),
            static_cast<float> ( _mouseNow.x - _mouseCommit.x )
        )
    );

    _mouseCommit = _mouseNow;

    _eulerAngles._data[ 0U ] = GXClampf ( _eulerAngles._data[ 0U ], -GX_MATH_HALF_PI, GX_MATH_HALF_PI );

    float yaw = _eulerAngles._data[ 1U ];
    constexpr float mirrorCases[] = { 1.0F, -1.0F };
    float const mirroring = mirrorCases[ static_cast<size_t> ( yaw < 0.0F ) ];
    yaw *= mirroring;

    while ( yaw > GX_MATH_DOUBLE_PI )
        yaw -= GX_MATH_DOUBLE_PI;

    _eulerAngles._data[ 1U ] = yaw * mirroring;

    GXQuat pitchFactor {};
    pitchFactor.FromAxisAngle ( GXVec3::RIGHT, _eulerAngles._data[ 0U ] );

    GXQuat yawFactor {};
    yawFactor.FromAxisAngle ( GXVec3::UP, _eulerAngles._data[ 1U ] );

    _orientation.Multiply ( yawFactor, pitchFactor );

    if ( !( _state._right | _state._left | _state._forward | _state._backward ) )
        return;

    constexpr float directionCases[] = { 0.0F, 1.0F };
    GXVec3 displacementLocal = GXVec3::ZERO;
    displacementLocal._data[ 0U ] = directionCases[ static_cast<size_t> ( _state._right ) ];
    displacementLocal._data[ 0U ] += -directionCases[ static_cast<size_t> ( _state._left ) ];
    displacementLocal._data[ 2U ] = directionCases[ static_cast<size_t> ( _state._forward ) ];
    displacementLocal._data[ 2U ] += -directionCases[ static_cast<size_t> ( _state._backward ) ];

    float const l = displacementLocal.SquaredLength ();

    // For example the user is pressing A+D or W+S.
    if ( l < MOVE_SPEED_THRESHOLD ) [[unlikely]]
        return;

    constexpr float speedCases[] = { FREE_FLY_MOVE_SPEED, FREE_FLY_SPRINT_SPEED };

    displacementLocal.Multiply ( displacementLocal,
        deltaTime * speedCases[ static_cast<size_t> ( _state._shift ) ] / std::sqrt ( l )
    );

    GXVec3 displacementWorld {};
    _orientation.TransformFast ( displacementWorld, displacementLocal );
    _location.Sum ( _location, displacementWorld );
}

void ViewportWidget::DoOrbit () noexcept
{
    // FUCK
}

void ViewportWidget::SwitchTool ( Tool &tool ) noexcept
{
    Tool* old = std::exchange ( _activeTool, &tool );

    if ( old == &tool ) [[unlikely]]
        return;

    if ( old ) [[likely]]
        old->Deactivate ();

    tool.Activate ();
}

} // namespace editor
