#include <precompiled_headers.hpp>
#include "theme.hpp"
#include <ui_label.hpp>


namespace editor {

namespace {

struct SetTextEvent final
{
    std::string             _text {};
    pbr::TextUIElement*     _ui = nullptr;
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UILabel::UILabel ( MessageQueue &messageQueue, pbr::UIElement const &parent, std::string_view text ) noexcept:
    _messageQueue ( messageQueue )
{
    _div = std::make_unique<pbr::DIVUIElement> ( &parent,

        pbr::CSSComputedValues
        {
            ._backgroundColor = pbr::ColorValue ( false, theme::TRANSPARENT_COLOR ),
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = pbr::ColorValue ( false, theme::TEXT_COLOR_NORMAL ),
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::FONT_SIZE,
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
            ._width = theme::ZERO_LENGTH,
            ._height = theme::ZERO_LENGTH
        }
    );

    pbr::DIVUIElement &div = *_div;
    _text = std::make_unique<pbr::TextUIElement> ( true, &div, text );
    div.AppendChildElement ( *_text );
}

void UILabel::operator = ( std::string &&text ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UILabelSetText,

            ._params = new SetTextEvent
            {
                ._text = std::move ( text ),
                ._ui = _text.get ()
            },

            ._serialNumber = 0U
        }
    );
}

pbr::DIVUIElement& UILabel::GetDIV () noexcept
{
    return *_div;
}

void UILabel::OnSetText ( Message &&message ) noexcept
{
    auto const* event = static_cast<SetTextEvent const*> ( message._params );
    std::string const &text = event->_text;
    event->_ui->SetText ( std::string_view ( text.data (), text.size () ) );
    delete event;
}

} // namespace editor
