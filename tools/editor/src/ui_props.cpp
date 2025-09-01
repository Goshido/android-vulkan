#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <theme.hpp>
#include <ui_props.hpp>


namespace editor {

namespace {

constexpr UIComboBox::ID R1920x1080 = 0U;
constexpr UIComboBox::ID R1680x1050 = 1U;
constexpr UIComboBox::ID R1600x1024 = 2U;
constexpr UIComboBox::ID R1600x900 = 3U;

constexpr UIComboBox::Item const RESOLUTIONS[] =
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

// FUCK - remove namespace
UIProps::UIProps ( MessageQueue &messageQueue, pbr::android::FontStorage &fontStorage ) noexcept:
    UIDialogBox ( messageQueue, "Properties" ),
    _fontStorage ( fontStorage ),

    _headerLine ( messageQueue,
        _div,

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
            ._marginBottom = pbr::LengthValue ( pbr::LengthValue::eType::PX, 2.0F ),
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

    _headerText ( messageQueue, _headerLine, "Properties", "Header"),
    _closeButton ( messageQueue, _headerLine, "Close button" ),
    _checkBox ( messageQueue, _div, "Shadows", "CheckBox" ),
    _comboBox ( messageQueue, _div, "Resolution", { RESOLUTIONS, std::size ( RESOLUTIONS ) }, R1600x1024, "ComboBox" ),
    _slider ( messageQueue, _div, "Blur", 0.0, 1.0, 0.1, 0.5, "Slider" ),
    _editBox ( messageQueue, _div, fontStorage, "Name", "The quick brown fox jumps", "EditBox" )
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
    _checkBox.Connect ( std::bind ( &UIProps::OnCheckBox, this, std::placeholders::_1 ) );
    _comboBox.Connect ( std::bind ( &UIProps::OnComboBox, this, std::placeholders::_1 ) );
    _editBox.Connect ( std::bind ( &UIProps::OnEditBox, this, std::placeholders::_1 ) );
    _slider.Connect ( std::bind ( &UIProps::OnSlider, this, std::placeholders::_1 ) );

    _div.PrependChildElement ( _headerLine );
}

void UIProps::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonDown ( event );
        return;
    }

    if ( _checkBox.IsOverlapped ( event._x, event._y ) )
    {
        _checkBox.OnMouseButtonDown ( event );
        return;
    }

    if ( _comboBox.IsOverlapped ( event._x, event._y ) )
    {
        _comboBox.OnMouseButtonDown ( event );
        return;
    }

    if ( _slider.IsOverlapped ( event._x, event._y ) )
    {
        _slider.OnMouseButtonDown ( event );
        return;
    }

    if ( _editBox.IsOverlapped ( event._x, event._y ) )
    {
        _editBox.OnMouseButtonDown ( event );
        return;
    }

    UIDialogBox::OnMouseButtonDown ( event );
}

void UIProps::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonUp ( event );
        return;
    }

    if ( _checkBox.IsOverlapped ( event._x, event._y ) )
    {
        _checkBox.OnMouseButtonUp ( event );
        return;
    }

    if ( _comboBox.IsOverlapped ( event._x, event._y ) )
    {
        _comboBox.OnMouseButtonUp ( event );
        return;
    }

    if ( _slider.IsOverlapped ( event._x, event._y ) )
    {
        _slider.OnMouseButtonUp ( event );
        return;
    }

    if ( _editBox.IsOverlapped ( event._x, event._y ) )
    {
        _editBox.OnMouseButtonUp ( event );
        return;
    }

    UIDialogBox::OnMouseButtonUp ( event );
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

    if ( _checkBox.IsOverlapped ( event._x, event._y ) )
    {
        _checkBox.OnMouseMove ( event );
        return;
    }

    if ( _comboBox.IsOverlapped ( event._x, event._y ) )
    {
        _comboBox.OnMouseMove ( event );
        return;
    }

    if ( _slider.IsOverlapped ( event._x, event._y ) )
    {
        _slider.OnMouseMove ( event );
        return;
    }

    if ( _editBox.IsOverlapped ( event._x, event._y ) )
    {
        _editBox.OnMouseMove ( event );
        return;
    }

    UIDialogBox::OnMouseMove ( event );
}

// FUCK - remove namespace
void UIProps::Submit ( pbr::android::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );
    _closeButton.UpdatedRect ();
    _checkBox.UpdatedRect ();
    _comboBox.UpdatedRect ();
    _slider.UpdatedRect ();
    _editBox.UpdatedRect ();
}

void UIProps::OnCheckBox ( UICheckBox::eState state ) noexcept
{
    android_vulkan::LogDebug ( "OnCheckBox - %hhu", static_cast<uint8_t> ( state ) );
}

void UIProps::OnClose () noexcept
{
    android_vulkan::LogDebug ( "OnClose" );
}

void UIProps::OnComboBox ( UIComboBox::ID id ) noexcept
{
    android_vulkan::LogDebug ( "OnComboBox - %u", id );
}

void UIProps::OnEditBox ( std::string const &value ) noexcept
{
    android_vulkan::LogDebug ( "OnEditBox - %s", value.c_str () );
}

void UIProps::OnSlider ( double value ) noexcept
{
    android_vulkan::LogDebug ( "OnSlider - %g", value );
}

} // namespace editor
