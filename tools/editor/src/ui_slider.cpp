#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <cursor.hpp>
#include <theme.hpp>
#include <ui_slider.hpp>

namespace editor {

UISlider::UISlider ( MessageQueue &messageQueue,
    DIVUIElement &parent,
    std::string_view caption,
    double minValue,
    double maxValue,
    double value,
    double defaultValue,
    std::string &&name
) noexcept:
    Widget ( messageQueue ),

    _lineDIV ( messageQueue,
        parent,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = theme::ZERO_LENGTH,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = pbr::LengthValue ( pbr::LengthValue::eType::PX, 9.0F ),
            ._paddingLeft = pbr::LengthValue ( pbr::LengthValue::eType::PX, 6.0F ),
            ._paddingRight = pbr::LengthValue ( pbr::LengthValue::eType::PX, 6.0F ),
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = theme::AUTO_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F )
        },

        name + " (line)"
    ),

    _columnDIV ( messageQueue,
        _lineDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::AUTO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = theme::INHERIT_LENGTH,
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

        name + " (column)"
    ),

    _captionDIV ( messageQueue,
        _columnDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::AUTO_LENGTH,
            ._color = theme::TEXT_COLOR_NORMAL,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile { theme::NORMAL_FONT_FAMILY },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 35.0F ),
            ._height = theme::AUTO_LENGTH
        },

        name + " (caption)"
    ),

    _captionText ( messageQueue, _captionDIV, caption, name + " (caption)" ),

    _valueDIV ( messageQueue,
        _columnDIV,

        {
            ._backgroundColor = theme::WIDGET_BACKGROUND_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile {},
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Center,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 65.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (value)"
    ),

    _progressDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = pbr::ColorValue ( 0U, 0U, 0U, 64U ),
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = theme::INHERIT_LENGTH,
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
            ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 10.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (progress)"
    ),

    _numberDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._position = pbr::PositionProperty::eValue::Absolute,
            ._textAlign = pbr::TextAlignProperty::eValue::Center,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (number)"
    ),

    _number ( messageQueue, _numberDIV, "", name + " (number)" ),
    _minValue ( minValue ),
    _maxValue ( maxValue ),
    _range ( maxValue - minValue ),
    _value ( minValue - 42.0 ),
    _defaultValue ( defaultValue )
{
    AV_ASSERT ( minValue < maxValue && defaultValue >= minValue && defaultValue <= maxValue )

    _captionDIV.AppendChildElement ( _captionText );
    _columnDIV.AppendChildElement ( _captionDIV );

    _valueDIV.AppendChildElement ( _progressDIV );

    _numberDIV.AppendChildElement ( _number );
    _valueDIV.AppendChildElement ( _numberDIV );

    _columnDIV.AppendChildElement ( _valueDIV );

    _lineDIV.AppendChildElement ( _columnDIV );
    parent.AppendChildElement ( _lineDIV );

    SwitchToNormalState ();
    UpdateProgress ( value / _range );
}

void UISlider::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    ( this->*_onMouseKeyDown ) ( event );
}

void UISlider::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    ( this->*_onMouseKeyUp ) ( event );
}

void UISlider::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    ( this->*_onMouseMove ) ( event );
}

void UISlider::UpdatedRect () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
}

void UISlider::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

void UISlider::OnMouseLeave () noexcept
{
    _number.SetColor ( theme::MAIN_COLOR );
}

void UISlider::OnMouseButtonDownDrag ( MouseButtonEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

void UISlider::OnMouseButtonUpDrag ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StopWidgetCaptureInput,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );

    --_eventID;
    SwitchToNormalState ();
}

void UISlider::OnMouseMoveDrag ( MouseMoveEvent const &event ) noexcept
{
    UpdateValue ( event._x );
}

void UISlider::OnMouseButtonDownNormal ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key == eKey::RightMouseButton ) [[unlikely]]
    {
        UpdateProgress ( _defaultValue / _range );
        return;
    }

    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _number.SetColor ( theme::PRESS_COLOR );
    UpdateValue ( event._x );
    SwitchToDragState ();
}

void UISlider::OnMouseButtonUpNormal ( MouseButtonEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

void UISlider::OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ChangeCursor,
            ._params = reinterpret_cast<void*> ( eCursor::Arrow ),
            ._serialNumber = 0U
        }
    );

    _number.SetColor ( theme::HOVER_COLOR );
}

void UISlider::SwitchToDragState () noexcept
{
    _onMouseKeyDown = &UISlider::OnMouseButtonDownDrag;
    _onMouseKeyUp = &UISlider::OnMouseButtonUpDrag;
    _onMouseMove = &UISlider::OnMouseMoveDrag;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StartWidgetCaptureInput,
            ._params = this,
            ._serialNumber = 0U
        }
    );
}

void UISlider::SwitchToNormalState () noexcept
{
    _onMouseKeyDown = &UISlider::OnMouseButtonDownNormal;
    _onMouseKeyUp = &UISlider::OnMouseButtonUpNormal;
    _onMouseMove = &UISlider::OnMouseMoveNormal;
}

void UISlider::UpdateProgress ( double progress ) noexcept
{
    if ( double const old = std::exchange ( _value, _minValue + _range * progress ); old == _value ) [[unlikely]]
        return;

    if ( _callback ) [[likely]]
        _callback ( _value );

    _progressDIV.GetCSS ()._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent,
        100.0F * static_cast<float> ( progress )
    );

    char buf[ 128U ];
    _number.SetText ( { buf, static_cast<size_t> ( std::snprintf ( buf, std::size ( buf ), "%.05f", _value ) ) } );
}

void UISlider::UpdateValue ( int32_t mouseX ) noexcept
{
    int32_t const x = std::clamp ( mouseX, _rect._left, _rect._right );
    UpdateProgress ( static_cast<double> ( x - _rect._left ) / static_cast<double> ( _rect.GetWidth () ) );
}

} // namespace editor
