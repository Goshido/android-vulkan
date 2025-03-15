#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <theme.hpp>
#include <ui_props.hpp>


namespace editor {

namespace {

constexpr UICombobox::ID R1920x1080 = 0U;
constexpr UICombobox::ID R1680x1050 = 1U;
constexpr UICombobox::ID R1600x1024 = 2U;
constexpr UICombobox::ID R1600x900 = 3U;

constexpr UICombobox::Item const RESOLUTIONS[] =
{
    {
        ._caption = "1920x1080",
        ._id = R1920x1080
    },
    {
        ._caption = "1680x1050",
        ._id = R1680x1050
    },
    {
        ._caption = "1600x1024",
        ._id = R1600x1024
    },
    {
        ._caption = "1600x900",
        ._id = R1600x900
    }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UIProps::UIProps ( MessageQueue &messageQueue ) noexcept:
    UIDialogBox ( messageQueue, "Properties" ),

    _headerLine ( &_div,

        pbr::CSSComputedValues
        {
            ._backgroundColor = theme::HEADER_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::HEADER_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width =  pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = theme::HEADER_HEIGHT
        },

        "Header line"
    ),

    _headerText ( messageQueue, _headerLine, "Properties", "Header" ),
    _closeButton ( messageQueue, _headerLine, "Close button" ),
    _checkbox ( messageQueue, _div, "Shadows", "Checkbox" ),
    _combobox ( messageQueue, _div, "Resolution", { RESOLUTIONS, std::size ( RESOLUTIONS ) }, R1600x1024, "Combobox" )
{
    pbr::CSSComputedValues &headerTextStyle = _headerText.GetCSS ();
    headerTextStyle._fontSize = theme::HEADER_FONT_SIZE;
    headerTextStyle._textAlign = pbr::TextAlignProperty::eValue::Center;
    headerTextStyle._paddingTop = theme::HEADER_VERTICAL_PADDING;
    headerTextStyle._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );

    pbr::CSSComputedValues &closeButtonStyle = _closeButton.GetCSS ();
    closeButtonStyle._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );
    closeButtonStyle._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );

    _closeButton.Connect ( std::bind ( &UIProps::OnClose, this ) );
    _checkbox.Connect ( std::bind ( &UIProps::OnCheckBox, this, std::placeholders::_1 ) );
    _combobox.Connect ( std::bind ( &UIProps::OnCombobox, this, std::placeholders::_1 ) );

    _div.PrependChildElement ( _headerLine );
}

void UIProps::OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseKeyDown ( event );
        return;
    }

    if ( _checkbox.IsOverlapped ( event._x, event._y ) )
    {
        _checkbox.OnMouseKeyDown ( event );
        return;
    }

    if ( _combobox.IsOverlapped ( event._x, event._y ) )
    {
        _combobox.OnMouseKeyDown ( event );
        return;
    }

    UIDialogBox::OnMouseKeyDown ( event );
}

void UIProps::OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseKeyUp ( event );
        return;
    }

    if ( _checkbox.IsOverlapped ( event._x, event._y ) )
    {
        _checkbox.OnMouseKeyUp ( event );
        return;
    }

    if ( _combobox.IsOverlapped ( event._x, event._y ) )
    {
        _combobox.OnMouseKeyUp ( event );
        return;
    }

    UIDialogBox::OnMouseKeyUp ( event );
}

void UIProps::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    if ( _dragState )
    {
        UIDialogBox::OnMouseMove ( event );
        return;
    }

    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseMove ( event );
        return;
    }

    if ( _checkbox.IsOverlapped ( event._x, event._y ) )
    {
        _checkbox.OnMouseMove ( event );
        return;
    }

    if ( _combobox.IsOverlapped ( event._x, event._y ) )
    {
        _combobox.OnMouseMove ( event );
        return;
    }

    UIDialogBox::OnMouseMove ( event );
}

void UIProps::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );
    _closeButton.UpdatedRect ();
    _checkbox.UpdatedRect ();
    _combobox.UpdatedRect ();
}

void UIProps::OnCheckBox ( UICheckbox::eState state ) noexcept
{
    android_vulkan::LogDebug ( "OnCheckBox - %hhu", static_cast<uint8_t> ( state ) );
}

void UIProps::OnClose () noexcept
{
    android_vulkan::LogDebug ( "OnClose" );
}

void UIProps::OnCombobox ( UICombobox::ID id ) noexcept
{
    android_vulkan::LogDebug ( "OnCombobox - %u", id );
}

} // namespace editor
