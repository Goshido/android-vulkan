#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <theme.hpp>
#include <viewport_widget.hpp>


namespace editor {

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

void ViewportWidget::Update ( float /*deltaTime*/ ) noexcept
{
    eNavigationMode const old = _navigationMode;
    ResolveNavigationMode ();

    if ( old == _navigationMode ) [[likely]]
        return;

    switch ( _navigationMode )
    {
        case eNavigationMode::FreeFly:
            android_vulkan::LogInfo ( ">>> Mode is FreeFly" );
        break;

        case eNavigationMode::Orbit:
            android_vulkan::LogInfo ( ">>> Mode is Orbit" );
        break;

        case eNavigationMode::None:
            android_vulkan::LogInfo ( ">>> Mode is None" );
        break;

        default:
            // IMPOSSIBLE
        break;
    }
}

void ViewportWidget::OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept
{
    UpdateKeyboardState ( key, modifier, 1U );
}

void ViewportWidget::OnKeyboardKeyUp ( eKey key, KeyModifier modifier ) noexcept
{
    UpdateKeyboardState ( key, modifier, 0U );
}

void ViewportWidget::OnMouseLeave () noexcept
{
    // FUCK
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
    // FUCK
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
    _aspectRatio = size._data[ 0U ] / size._data[ 1U ];

    UpdateCamera ();

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

void ViewportWidget::UpdateCamera () noexcept
{
    android_vulkan::LogInfo ( ">>> UpdateCamera" );
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

    _navigationMode = resultCases[
        static_cast<size_t> ( ( _navigationMode == eNavigationMode::None ) | ( current == eNavigationMode::None ) )
    ];
}

} // namespace editor
