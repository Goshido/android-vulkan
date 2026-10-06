#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <theme.hpp>
#include <ui_transform.hpp>


namespace editor {

namespace {

constexpr pbr::LengthValue MIN_WIDTH ( pbr::LengthValue::eType::PX, 155.0F );

// FUCK - figure out why min height must add header height
//constexpr pbr::LengthValue MIN_HEIGHT ( pbr::LengthValue::eType::PX, 203.0F );
constexpr pbr::LengthValue MIN_HEIGHT ( pbr::LengthValue::eType::PX, 233.0F );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UITransform::UITransform () noexcept:
    UIDialogBox ( "Transform" ),

    _headerLine ( _div,

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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = theme::HEADER_HEIGHT
        },

        "Header line"
    ),

    _headerText ( _headerLine, "Transform", "Header"),
    _closeButton ( _headerLine, "Close button" ),
    _locationX ( _div, "X", "0", "EditBox[x]" ),
    _locationY ( _div, "Y", "0", "EditBox[y]" ),
    _locationZ ( _div, "Z", "0", "EditBox[z]" ),
    _separator ( _div, "Separator" ),
    _scaleX ( _div, "Scale X", "0", "EditBox[scale-x]" ),
    _scaleY ( _div, "Scale Y", "0", "EditBox[scale-y]" ),
    _scaleZ ( _div, "Scale Z", "0", "EditBox[scale-z]" )
{
    pbr::CSSComputedValues &headerTextStyle = _headerText.GetCSS ();
    headerTextStyle._fontSize = theme::HEADER_FONT_SIZE;
    headerTextStyle._textAlign = pbr::TextAlignProperty::eValue::Center;
    headerTextStyle._paddingTop = theme::HEADER_VERTICAL_PADDING;
    headerTextStyle._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );

    pbr::CSSComputedValues &closeButtonStyle = _closeButton.GetCSS ();
    closeButtonStyle._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );
    closeButtonStyle._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );

    _closeButton.Connect ( std::bind ( &UITransform::OnClose, this ) );
    _locationX.Connect ( std::bind ( &UITransform::OnLocationX, this, std::placeholders::_1 ) );
    _locationY.Connect ( std::bind ( &UITransform::OnLocationY, this, std::placeholders::_1 ) );
    _locationZ.Connect ( std::bind ( &UITransform::OnLocationZ, this, std::placeholders::_1 ) );
    _scaleX.Connect ( std::bind ( &UITransform::OnScaleX, this, std::placeholders::_1 ) );
    _scaleY.Connect ( std::bind ( &UITransform::OnScaleY, this, std::placeholders::_1 ) );
    _scaleZ.Connect ( std::bind ( &UITransform::OnScaleZ, this, std::placeholders::_1 ) );

    _div.PrependChildElement ( _headerLine );
    SetMinSize ( MIN_WIDTH, MIN_HEIGHT );
}

void UITransform::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonDown ( event );
        return;
    }

    if ( _locationX.IsOverlapped ( event._x, event._y ) )
    {
        _locationX.OnMouseButtonDown ( event );
        return;
    }

    if ( _locationY.IsOverlapped ( event._x, event._y ) )
    {
        _locationY.OnMouseButtonDown ( event );
        return;
    }

    if ( _locationZ.IsOverlapped ( event._x, event._y ) )
    {
        _locationZ.OnMouseButtonDown ( event );
        return;
    }

    if ( _scaleX.IsOverlapped ( event._x, event._y ) )
    {
        _scaleX.OnMouseButtonDown ( event );
        return;
    }

    if ( _scaleY.IsOverlapped ( event._x, event._y ) )
    {
        _scaleY.OnMouseButtonDown ( event );
        return;
    }

    if ( _scaleZ.IsOverlapped ( event._x, event._y ) )
    {
        _scaleZ.OnMouseButtonDown ( event );
        return;
    }

    UIDialogBox::OnMouseButtonDown ( event );
}

void UITransform::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonUp ( event );
        return;
    }

    if ( _locationX.IsOverlapped ( event._x, event._y ) )
    {
        _locationX.OnMouseButtonUp ( event );
        return;
    }

    if ( _locationY.IsOverlapped ( event._x, event._y ) )
    {
        _locationY.OnMouseButtonUp ( event );
        return;
    }

    if ( _locationZ.IsOverlapped ( event._x, event._y ) )
    {
        _locationZ.OnMouseButtonUp ( event );
        return;
    }

    if ( _scaleX.IsOverlapped ( event._x, event._y ) )
    {
        _scaleX.OnMouseButtonUp ( event );
        return;
    }

    if ( _scaleY.IsOverlapped ( event._x, event._y ) )
    {
        _scaleY.OnMouseButtonUp ( event );
        return;
    }

    if ( _scaleZ.IsOverlapped ( event._x, event._y ) )
    {
        _scaleZ.OnMouseButtonUp ( event );
        return;
    }

    UIDialogBox::OnMouseButtonUp ( event );
}

void UITransform::OnMouseMove ( MouseMoveEvent const &event ) noexcept
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

    if ( _locationX.IsOverlapped ( event._x, event._y ) )
    {
        _locationX.OnMouseMove ( event );
        return;
    }

    if ( _locationY.IsOverlapped ( event._x, event._y ) )
    {
        _locationY.OnMouseMove ( event );
        return;
    }

    if ( _locationZ.IsOverlapped ( event._x, event._y ) )
    {
        _locationZ.OnMouseMove ( event );
        return;
    }

    if ( _scaleX.IsOverlapped ( event._x, event._y ) )
    {
        _scaleX.OnMouseMove ( event );
        return;
    }

    if ( _scaleY.IsOverlapped ( event._x, event._y ) )
    {
        _scaleY.OnMouseMove ( event );
        return;
    }

    if ( _scaleZ.IsOverlapped ( event._x, event._y ) )
    {
        _scaleZ.OnMouseMove ( event );
        return;
    }

    UIDialogBox::OnMouseMove ( event );
}

void UITransform::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );
    _closeButton.UpdatedRect ();
    _locationX.UpdatedRect ();
    _locationY.UpdatedRect ();
    _locationZ.UpdatedRect ();
    _scaleX.UpdatedRect ();
    _scaleY.UpdatedRect ();
    _scaleZ.UpdatedRect ();
}

void UITransform::OnClose () noexcept
{
    android_vulkan::LogDebug ( "OnClose" );
}

void UITransform::OnLocationX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "OnLocationX" );
}

void UITransform::OnLocationY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "OnLocationY" );
}

void UITransform::OnLocationZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "OnLocationZ" );
}

void UITransform::OnScaleX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "OnScaleX" );
}

void UITransform::OnScaleY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "OnScaleY" );
}

void UITransform::OnScaleZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "OnScaleZ" );
}

} // namespace editor
