#include <precompiled_headers.hpp>
#include <theme.hpp>
#include <ui_props.hpp>


namespace editor {

UIProps::UIProps ( MessageQueue &messageQueue ) noexcept:
    UIDialogBox ( messageQueue ),
    _header ( std::make_unique<UILabel> ( messageQueue, _div, "Properties" ) )
{
    pbr::DIVUIElement &headerDiv = _header->GetDIV ();
    pbr::CSSComputedValues &h = headerDiv.GetCSS ();
    h._backgroundColor = theme::HEADER_COLOR;
    h._textAlign = pbr::TextAlignProperty::eValue::Center;
    h._paddingTop = theme::HEADER_VERTICAL_PADDING;
    h._paddingBottom = theme::HEADER_VERTICAL_PADDING;
    h._paddingLeft = theme::ZERO_LENGTH;
    h._paddingRight = theme::ZERO_LENGTH;
    h._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );
    h._height = theme::AUTO_LENGTH;

    _div.AppendChildElement ( headerDiv );
}

} // namespace editor
