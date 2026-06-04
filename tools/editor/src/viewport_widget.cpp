#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <theme.hpp>
#include <viewport_widget.hpp>

// FUCK
#include <scope_quard.hpp>


namespace editor {

namespace {

constexpr float FREE_FLY_ORIENTATION_SPEED = 4.0e-2F;
constexpr float FREE_FLY_MOVE_SPEED = 1.0F;
constexpr float FREE_FLY_SPRINT_SPEED = 10.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+3F;
constexpr float FOV_Y = GXDegToRad ( 70.0F );

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
    )
{
    // NOTHING
}

void ViewportWidget::Update ( float deltaTime, float dpi ) noexcept
{
    ResolveNavigationMode ();

    switch ( _navigationMode )
    {
        case eNavigationMode::FreeFly:
            DoFreeFly ( deltaTime, dpi );
        break;

        case eNavigationMode::Orbit:
            DoOrbit ();
        break;

        case eNavigationMode::None:
            // NOTHING
            [[fallthrough]];
        default:
            // IMPOSSIBLE
        break;
    }
}

GXMat4 const &ViewportWidget::GetProjection () const noexcept
{
    return _projection;
}

GXQuat const &ViewportWidget::GetOrientation () const noexcept
{
    return _orientation;
}

GXVec3 const &ViewportWidget::GetPosition () const noexcept
{
    return _position;
}

void ViewportWidget::OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept
{
    UpdateKeyboardState ( key, modifier, 1U );
}

void ViewportWidget::OnKeyboardKeyUp ( eKey key, KeyModifier modifier ) noexcept
{
    UpdateKeyboardState ( key, modifier, 0U );
}

void ViewportWidget::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    UpdateMouseState ( event, 1U );
}

void ViewportWidget::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    UpdateMouseState ( event, 0U );
}

void ViewportWidget::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) > 1U ) [[unlikely]]
        ChangeCursor ( eCursor::Arrow );

    _mouseNow =
    {
        ._x = event._x,
        ._y = event._y
    };
}

Widget::LayoutStatus ViewportWidget::ApplyLayout ( android_vulkan::Renderer &renderer,
    pbr::FontStorage &fontStorage
) noexcept
{
    VkExtent2D const viewport = renderer.GetViewportResolution ();

    if ( ( viewport.width == _resolution.width ) & ( viewport.height == _resolution.height ) ) [[likely]]
    {
        return
        {
            ._hasChanges = false,
            ._neededUIVertices = 0U
        };
    }

    _resolution = viewport;

    GXVec2 const size ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) );
    _projection.Perspective ( FOV_Y, size._data[ 0U ] / size._data[ 1U ], Z_NEAR, Z_FAR );

    // It's needed to update widget boundaries according to HTML+CSS settings.
    // The viewport widget itself does not have any child elements.
    // Note that bounds are updated in DIVUIElement::UpdateCache method.

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

    pbr::UIElement::UpdateInfo updateInfo
    {
        ._fontStorage = &fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = size,
        ._parentTopLeft = GXVec2::ZERO,
        ._pen = GXVec2::ZERO
    };

    std::ignore = _div.UpdateCache ( updateInfo );
    _rect.From ( _div.GetAbsoluteRect () );

    return
    {
        ._hasChanges = false,
        ._neededUIVertices = 0U
    };
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
    _state._shift = static_cast<uint8_t> ( modifier.AnyShiftPressed () );
    _state._alt = static_cast<uint8_t> ( modifier.AnyAltPressed () );

    _mouseNow =
    {
        ._x = event._x,
        ._y = event._y
    };
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

    Mouse const mouseCases[] = { _mouseCommit, _mouseNow };
    _mouseCommit = mouseCases[ static_cast<size_t> ( old != _navigationMode ) ];
}

void ViewportWidget::DoFreeFly ( float deltaTime, float dpi ) noexcept
{
    _eulerAngles.Sum ( _eulerAngles,
        FREE_FLY_ORIENTATION_SPEED / dpi,

        GXVec2 ( static_cast<float> ( _mouseNow._y - _mouseCommit._y ),
            static_cast<float> ( _mouseNow._x - _mouseCommit._x )
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

    // FUCK
    android_vulkan::ScopeGuard const fuck (
        [ this ] () noexcept {
            android_vulkan::LogInfo ( "%g %g %g", _position._data[ 0U ], _position._data[ 1U ], _position._data[ 2U ] );
        }
    );

    if ( !( _state._right | _state._left | _state._forward | _state._backward ) )
        return;

    constexpr float directionCases[] = { 0.0F, 1.0F };
    GXVec3 displacementLocal = GXVec3::ZERO;
    displacementLocal._data[ 0U ] = directionCases[ static_cast<size_t> ( _state._right ) ];
    displacementLocal._data[ 0U ] += -directionCases[ static_cast<size_t> ( _state._left ) ];
    displacementLocal._data[ 2U ] = directionCases[ static_cast<size_t> ( _state._forward ) ];
    displacementLocal._data[ 2U ] += -directionCases[ static_cast<size_t> ( _state._backward ) ];

    constexpr float speedCases[] = { FREE_FLY_MOVE_SPEED, FREE_FLY_SPRINT_SPEED };

    displacementLocal.Multiply ( displacementLocal,
        deltaTime * speedCases[ static_cast<size_t> ( _state._shift ) ] / displacementLocal.Length ()
    );

    GXVec3 displacementWorld {};
    _orientation.TransformFast ( displacementWorld, displacementLocal );
    _position.Sum ( _position, displacementWorld );
}

void ViewportWidget::DoOrbit () noexcept
{
    // FUCK
}

} // namespace editor
