#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <cursor.hpp>
#include <glyphs.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <theme.hpp>
#include <ui_combo_box.hpp>


namespace editor {

namespace {

constexpr pbr::ColorValue POPUP_BACKGROUND ( 0U, 0U, 0U, 191U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UIComboBox::MenuItem::MenuItem ( std::unique_ptr<DIVUIElement> &&div,
    std::unique_ptr<TextUIElement> &&text
) noexcept:
    _div ( std::move ( div ) ),
    _text ( std::move ( text ) )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

UIComboBox::Popup::Popup ( MessageQueue &messageQueue,
    DIVUIElement const &positionAnchor,
    DIVUIElement const &widthAnchor,
    Items items,
    size_t &selected,
    TextUIElement &text,
    Callback &callback,
    std::string const &name
) noexcept:
    Widget ( messageQueue ),

    _div ( messageQueue,
        {
            ._backgroundColor = POPUP_BACKGROUND,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
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
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Absolute,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = theme::AUTO_LENGTH
        },

        name + " (menu)"
    ),

    _text ( text ),
    _callback ( callback ),
    _items ( items ),
    _selected ( selected )
{
    float const devicePXtoCSSPX = pbr::CSSUnitToDevicePixel::GetInstance ()._devicePXtoCSSPX;
    GXVec2 topLeft {};
    topLeft.Multiply ( positionAnchor.GetAbsoluteRect ()._topLeft, devicePXtoCSSPX );

    pbr::CSSComputedValues &css = _div.GetCSS ();
    css._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, topLeft._data[ 0U ] );
    css._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, topLeft._data[ 1U ] );

    pbr::DIVUIElement::Rect const &widthRect = widthAnchor.GetAbsoluteRect ();

    css._width = pbr::LengthValue ( pbr::LengthValue::eType::PX,
        devicePXtoCSSPX * ( widthRect._bottomRight._data[ 0U ] - widthRect._topLeft._data[ 0U ] )
    );

    size_t const count = items.size ();
    _menuItems.reserve ( count );

    for ( size_t i = 0U; i < count; ++i )
    {
        Item const &item = items[ i ];

        size_t const cases[] = { _selected, i };
        _selected = cases[ static_cast<size_t> ( item._id == selected ) ];

        std::unique_ptr<DIVUIElement> div = std::make_unique<DIVUIElement> ( messageQueue,
            _div,

            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::TRANSPARENT_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::AUTO_LENGTH,
                ._left = theme::AUTO_LENGTH,
                ._right = theme::AUTO_LENGTH,
                ._top = theme::AUTO_LENGTH,
                ._color = theme::MAIN_COLOR,
                ._display = pbr::DisplayProperty::eValue::Block,
                ._fontFile{},
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
                ._height = theme::MENU_ITEM_HEIGHT
            },

            name + " (item: " + item._caption.data () + ")"
        );

        std::unique_ptr<TextUIElement> t = std::make_unique<TextUIElement> ( messageQueue,
            *div,
            item._caption,
            name + " (item: " + item._caption.data () + ")"
        );

        div->AppendChildElement ( *t );
        _div.AppendChildElement ( *div );
        _menuItems.emplace_back ( std::move ( div ), std::move ( t ) );
    }

    if ( _selected == NO_INDEX ) [[unlikely]]
    {
        AV_ASSERT ( false )
        _selected = 0U;
    }

    Item const &item = items[ _selected ];
    _selected = item._id;
}

void UIComboBox::Popup::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    MenuItem* menuItems = _menuItems.data ();

    auto const isOverlap = [ &event, menuItems ] ( size_t idx ) noexcept -> bool {
        return Rect ( menuItems[ idx ]._div->GetAbsoluteRect () ).IsOverlapped ( event._x, event._y );
    };

    if ( _focused != NO_INDEX && isOverlap ( _focused ) ) [[likely]]
        return;

    size_t const oldFocused = std::exchange ( _focused, NO_INDEX );
    size_t const count = _items.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        if ( isOverlap ( i ) )
        {
            _focused = i;
            break;
        }
    }

    if ( oldFocused == _focused ) [[unlikely]]
        return;

    _targeted = NO_INDEX;

    auto const setColor = [ menuItems ] ( size_t idx, pbr::ColorValue const &color ) noexcept {
        if ( idx != NO_INDEX ) [[likely]]
        {
            menuItems[ idx ]._text->SetColor ( color );
        }
    };

    setColor ( oldFocused, theme::MAIN_COLOR );
    setColor ( _focused, theme::HOVER_COLOR );
    _isChanged = true;
}

Widget::LayoutStatus UIComboBox::Popup::ApplyLayout ( android_vulkan::Renderer &renderer,
    pbr::FontStorage &fontStorage
) noexcept
{
    VkExtent2D const viewport = renderer.GetViewportResolution ();

    _lineHeights.clear ();
    _lineHeights.push_back ( 0.0F );

    pbr::UIElement::ApplyInfo info
    {
        ._canvasSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._fontStorage = &fontStorage,
        ._hasChanges = _isChanged,
        ._lineHeights = &_lineHeights,
        ._parentPaddingExtent = GXVec2 ( 0.0F, 0.0F ),
        ._pen = GXVec2 ( 0.0F, 0.0F ),
        ._renderer = &renderer,
        ._vertices = 0U
    };

    _div.ApplyLayout ( info );
    _isChanged = false;

    return
    {
        ._hasChanges = info._hasChanges,
        ._neededUIVertices = info._vertices
    };
}

void UIComboBox::Popup::Submit ( pbr::UIElement::SubmitInfo& info ) noexcept
{
    _div.Submit ( info );
}

bool UIComboBox::Popup::UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept
{
    pbr::UIElement::UpdateInfo info
    {
        ._fontStorage = &fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._parentTopLeft = GXVec2 ( 0.0F, 0.0F ),
        ._pen = GXVec2 ( 0.0F, 0.0F )
    };

    return _div.UpdateCache ( info );
}

bool UIComboBox::Popup::HandleMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( !_rect.IsOverlapped ( event._x, event._y ) ) [[unlikely]]
        return true;

    _targeted = _focused;
    _menuItems[ _focused ]._text->SetColor ( theme::PRESS_COLOR );
    _isChanged = true;
    return false;
}

bool UIComboBox::Popup::HandleMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton || _targeted == NO_INDEX ) [[unlikely]]
        return false;

    if ( _targeted != std::exchange ( _selected, _targeted ) ) [[likely]]
    {
        Item const &item = _items[ _targeted ];
        _text.SetText ( item._caption );
        _callback ( item._id );
    }

    return true;
}

Rect const &UIComboBox::Popup::HandleUpdatedRect () noexcept
{
    _rect.From ( _div.GetAbsoluteRect () );
    return _rect;
}

//----------------------------------------------------------------------------------------------------------------------

UIComboBox::UIComboBox ( MessageQueue &messageQueue,
    DIVUIElement &parent,
    std::string_view caption,
    Items items,
    uint32_t selected,
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
            ._right = theme::AUTO_LENGTH,
            ._top = theme::AUTO_LENGTH,
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
            ._height = theme::MENU_ITEM_HEIGHT
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
            ._left = theme::AUTO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::AUTO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
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
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 65.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (value)"
    ),

    _textDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::AUTO_LENGTH,
            ._color = theme::INHERIT_COLOR,
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
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (text)"
    ),

    _text ( messageQueue, _textDIV, "", name + " (text)" ),

    _iconDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 2.0F ),
            ._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._color = theme::INHERIT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { glyph::FONT_FAMILY },
            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::PX, 11.0F ),
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Absolute,
            ._textAlign = pbr::TextAlignProperty::eValue::Right,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 14.0F )
        },

        name + " (icon)"
    ),

    _icon ( messageQueue, _iconDIV, glyph::COMBOBOX_DOWN, name + " (icon)" ),

    _menuAnchorDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
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
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = theme::ZERO_LENGTH
        },

        name + " (menu anchor)"
    ),

    _name ( std::move ( name ) ),
    _items ( items )
{
    AV_ASSERT ( !items.empty () )

    _captionDIV.AppendChildElement ( _captionText );
    _columnDIV.AppendChildElement ( _captionDIV );

    _textDIV.AppendChildElement ( _text );
    _valueDIV.AppendChildElement ( _textDIV );

    _iconDIV.AppendChildElement ( _icon );
    _valueDIV.AppendChildElement ( _iconDIV );

    _valueDIV.AppendChildElement ( _menuAnchorDIV );

    _columnDIV.AppendChildElement ( _valueDIV );

    _lineDIV.AppendChildElement ( _columnDIV );
    parent.AppendChildElement ( _lineDIV );

    size_t const count = items.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        Item const &item = items[ i ];

        size_t const cases[] = { _selected, i };
        _selected = cases[ static_cast<size_t> ( item._id == selected ) ];
    }

    if ( _selected == NO_INDEX ) [[unlikely]]
    {
        AV_ASSERT ( false )
        _selected = 0U;
    }

    Item const &item = items[ _selected ];
    _text.SetText ( item._caption );
    _selected = item._id;

    SwitchToNormalState ();
}

void UIComboBox::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    ( this->*_onMouseKeyDown ) ( event );
}

void UIComboBox::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    ( this->*_onMouseKeyUp ) ( event );
}

void UIComboBox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    ( this->*_onMouseMove ) ( event );
}

void UIComboBox::UpdatedRect () noexcept
{
    ( this->*_updateRect ) ();
}

void UIComboBox::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

void UIComboBox::OnMouseLeave () noexcept
{
    _text.SetColor ( theme::MAIN_COLOR );
    _icon.SetColor ( theme::MAIN_COLOR );
}

void UIComboBox::OnMouseButtonDownMenu ( MouseButtonEvent const &event ) noexcept
{
    if ( !_popup->HandleMouseButtonDown ( event ) )
        return;

    if ( event._key == eKey::LeftMouseButton )
        _cancelNextLeftMouseButtonDownEvent = Rect ( _valueDIV.GetAbsoluteRect () ).IsOverlapped ( event._x, event._y );

    SwitchToNormalState ();
    --_eventID;
}

void UIComboBox::OnMouseButtonUpMenu ( MouseButtonEvent const &event ) noexcept
{
    if ( _popup->HandleMouseButtonUp ( event ) )
    {
        SwitchToNormalState ();
    }
}

void UIComboBox::OnMouseMoveMenu ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );
    _popup->OnMouseMove ( event );
}

void UIComboBox::UpdatedRectMenu () noexcept
{
    _rect = _popup->HandleUpdatedRect ();
}

void UIComboBox::OnMouseButtonDownNormal ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _text.SetColor ( theme::PRESS_COLOR );
    _icon.SetColor ( theme::PRESS_COLOR );
}

void UIComboBox::OnMouseButtonUpNormal ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton || std::exchange ( _cancelNextLeftMouseButtonDownEvent, false ) )
    {
        [[unlikely]]
        return;
    }

    _text.SetColor ( theme::HOVER_COLOR );
    _icon.SetColor ( theme::HOVER_COLOR );
    SwitchToMenuState ();
}

void UIComboBox::OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return;

    ChangeCursor ( eCursor::Arrow );

    _text.SetColor ( theme::HOVER_COLOR );
    _icon.SetColor ( theme::HOVER_COLOR );
}

void UIComboBox::UpdatedRectNormal () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
}

void UIComboBox::SwitchToNormalState () noexcept
{
    _icon.SetText ( glyph::COMBOBOX_DOWN );
    _valueDIV.GetCSS ()._backgroundColor = theme::WIDGET_BACKGROUND_COLOR;

    UpdatedRectNormal ();

    _onMouseKeyDown = &UIComboBox::OnMouseButtonDownNormal;
    _onMouseKeyUp = &UIComboBox::OnMouseButtonUpNormal;
    _onMouseMove = &UIComboBox::OnMouseMoveNormal;
    _updateRect = &UIComboBox::UpdatedRectNormal;

    if ( !_popup ) [[unlikely]]
        return;

    ReleaseMouse ();

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIRemoveWidget,
            ._params = std::exchange ( _popup, nullptr ),
            ._serialNumber = 0U
        }
    );
}

void UIComboBox::SwitchToMenuState () noexcept
{
    _icon.SetText ( glyph::COMBOBOX_UP );
    _text.SetColor ( theme::MAIN_COLOR );
    _icon.SetColor ( theme::MAIN_COLOR );
    _valueDIV.GetCSS ()._backgroundColor = POPUP_BACKGROUND;

    _popup = new Popup ( _messageQueue,
        _menuAnchorDIV,
        _valueDIV,
        _items,
        _selected,
        _text,
        _callback,
        _name
    );

    _onMouseKeyDown = &UIComboBox::OnMouseButtonDownMenu;
    _onMouseKeyUp = &UIComboBox::OnMouseButtonUpMenu;
    _onMouseMove = &UIComboBox::OnMouseMoveMenu;
    _updateRect = &UIComboBox::UpdatedRectMenu;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAddWidget,
            ._params = _popup,
            ._serialNumber = 0U
        }
    );

    CaptureMouse ();
}

} // namespace editor
