#include <precompiled_headers.hpp>
#include <theme.hpp>
#include <ui_label.hpp>


namespace editor {

//----------------------------------------------------------------------------------------------------------------------

UILabel::UILabel ( MessageQueue &messageQueue,
    DIVUIElement &parent,
    std::string_view text,
    std::string &&name
) noexcept:
    _div ( messageQueue,
        parent,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TEXT_COLOR_NORMAL,
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
            ._width = theme::ZERO_LENGTH,
            ._height = theme::ZERO_LENGTH
        },

        name + " (DIV)"
    ),

    _text ( messageQueue, _div, text, name + " (text)" ),
    _messageQueue ( messageQueue )
{
    _div.AppendChildElement ( _text );
    parent.AppendChildElement ( _div );
}

pbr::CSSComputedValues &UILabel::GetCSS () noexcept
{
    return _div.GetCSS ();
}

} // namespace editor
