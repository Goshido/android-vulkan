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
            ._fontFile = "",
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

void ViewportWidget::OnKeyboardKeyDown ( eKey /*key*/, KeyModifier /*modifier*/ ) noexcept
{
    // FUCK
}

void ViewportWidget::OnKeyboardKeyUp ( eKey /*key*/, KeyModifier /*modifier*/ ) noexcept
{
    // FUCK
}

void ViewportWidget::OnMouseLeave () noexcept
{
    // FUCK
}

void ViewportWidget::OnMouseButtonDown ( MouseButtonEvent const &/*event*/ ) noexcept
{
    // FUCK
}

void ViewportWidget::OnMouseButtonUp ( MouseButtonEvent const &/*event*/ ) noexcept
{
    // FUCK
}

void ViewportWidget::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );
    // FUCK
}

bool ViewportWidget::UpdateCache ( pbr::FontStorage &/*fontStorage*/, VkExtent2D const &viewport ) noexcept
{
    if ( ( viewport.width == _resolution.width ) & ( viewport.height == _resolution.height ) ) [[likely]]
        return false;

    _resolution = viewport;
    _aspectRatio = static_cast<float> ( viewport.width ) / static_cast<float> ( viewport.height );
    UpdateCamera ();

    return false;
}

void ViewportWidget::UpdateCamera () noexcept
{
    android_vulkan::LogInfo ( ">>> UpdateCamera" );
}

} // namespace editor
