#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <cursor.hpp>
#include <glyphs.hpp>
#include <theme.hpp>
#include <ui_combobox.hpp>


namespace editor {

UICombobox::MenuItem::MenuItem ( std::unique_ptr<DIVUIElement> &&div,
    std::unique_ptr<TextUIElement> &&text
) noexcept:
    _div ( std::move ( div ) ),
    _text ( std::move ( text ) )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

UICombobox::UICombobox ( MessageQueue &messageQueue,
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

    _menuDIV ( messageQueue,
        _menuAnchorDIV,

        {
            ._backgroundColor = theme::WIDGET_BACKGROUND_COLOR,
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

    _items ( items )
{
    AV_ASSERT ( !items.empty () )

    _captionDIV.AppendChildElement ( _captionText );
    _columnDIV.AppendChildElement ( _captionDIV );

    _textDIV.AppendChildElement ( _text );
    _valueDIV.AppendChildElement ( _textDIV );

    _iconDIV.AppendChildElement ( _icon );
    _valueDIV.AppendChildElement ( _iconDIV );

    _menuAnchorDIV.AppendChildElement ( _menuDIV );
    _valueDIV.AppendChildElement ( _menuAnchorDIV );

    _columnDIV.AppendChildElement ( _valueDIV );

    _lineDIV.AppendChildElement ( _columnDIV );
    parent.AppendChildElement ( _lineDIV );

    _menuItems.reserve ( items.size () );
    size_t const count = items.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        Item const &item = items[ i ];

        size_t const cases[] = { _selected, i };
        _selected = cases[ static_cast<size_t> ( item._id == selected ) ];

        std::unique_ptr<DIVUIElement> div = std::make_unique<DIVUIElement> ( messageQueue,
            _menuDIV,

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

        std::unique_ptr<TextUIElement> text = std::make_unique<TextUIElement> ( messageQueue,
            *div,
            item._caption,
            name + " (item: " + item._caption.data () + ")"
        );

        div->AppendChildElement ( *text );
        _menuDIV.AppendChildElement ( *div );
        _menuItems.emplace_back ( std::move ( div ), std::move ( text ) );
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

void UICombobox::OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept
{
    ( this->*_onMouseKeyDown ) ( event );
}

void UICombobox::OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept
{
    ( this->*_onMouseKeyUp ) ( event );
}

void UICombobox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    ( this->*_onMouseMove ) ( event );
}

void UICombobox::UpdatedRect () noexcept
{
    ( this->*_updateRect ) ();
}

void UICombobox::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

void UICombobox::OnMouseLeave () noexcept
{
    _text.SetColor ( theme::MAIN_COLOR );
    _icon.SetColor ( theme::MAIN_COLOR );
}

void UICombobox::OnMouseKeyDownMenu ( MouseKeyEvent const &event ) noexcept
{
    if ( _rect.IsOverlapped ( event._x, event._y ) ) [[likely]]
    {
        _targeted = _focused;
        _menuItems[ _focused ]._text->SetColor ( theme::PRESS_COLOR );
        return;
    }

    _icon.SetText ( glyph::COMBOBOX_DOWN );
    _menuDIV.Hide ();
    --_eventID;
}

void UICombobox::OnMouseKeyUpMenu ( MouseKeyEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    if ( !_menuDIV.IsVisible () ) [[unlikely]]
    {
        _messageQueue.EnqueueBack (
            {
                ._type = eMessageType::StopWidgetCaptureMouse,
                ._params = this,
                ._serialNumber = 0U
            }
        );

        SwitchToNormalState ();
        return;
    }

    if ( _targeted == NO_INDEX ) [[unlikely]]
        return;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StopWidgetCaptureMouse,
            ._params = this,
            ._serialNumber = 0U
        }
    );

    _menuItems[ _targeted ]._text->SetColor ( theme::MAIN_COLOR );

    if ( _targeted == _selected ) [[unlikely]]
    {
        SwitchToNormalState ();
        return;
    }

    _selected = _targeted;
    Item const &item = _items[ _targeted ];
    _text.SetText ( item._caption );

    if ( _callback ) [[likely]]
        _callback ( item._id );

    SwitchToNormalState ();
}

void UICombobox::OnMouseMoveMenu ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );
    MenuItem* menuItems = _menuItems.data ();

    auto const isOverlap = [ &event, menuItems ] ( size_t idx ) noexcept -> bool {
        return
            Rect ( menuItems[ idx ]._div->GetAbsoluteRect () ).IsOverlapped ( event._x, event._y );
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
}

void UICombobox::UpdatedRectMenu () noexcept
{
    _rect.From ( _menuDIV.GetAbsoluteRect () );
}

void UICombobox::OnMouseKeyDownNormal ( MouseKeyEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _text.SetColor ( theme::PRESS_COLOR );
    _icon.SetColor ( theme::PRESS_COLOR );
}

void UICombobox::OnMouseKeyUpNormal ( MouseKeyEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _text.SetColor ( theme::HOVER_COLOR );
    _icon.SetColor ( theme::HOVER_COLOR );
    SwitchToMenuState ();
}

void UICombobox::OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept
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

    _text.SetColor ( theme::HOVER_COLOR );
    _icon.SetColor ( theme::HOVER_COLOR );
}

void UICombobox::UpdatedRectNormal () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
}

void UICombobox::SwitchToNormalState () noexcept
{
    _menuDIV.Hide ();
    _icon.SetText ( glyph::COMBOBOX_DOWN );
    UpdatedRectNormal ();

    _onMouseKeyDown = &UICombobox::OnMouseKeyDownNormal;
    _onMouseKeyUp = &UICombobox::OnMouseKeyUpNormal;
    _onMouseMove = &UICombobox::OnMouseMoveNormal;
    _updateRect = &UICombobox::UpdatedRectNormal;
}

void UICombobox::SwitchToMenuState () noexcept
{
    _icon.SetText ( glyph::COMBOBOX_UP );
    _text.SetColor ( theme::MAIN_COLOR );
    _icon.SetColor ( theme::MAIN_COLOR );
    _focused = NO_INDEX;
    _targeted = NO_INDEX;

    _onMouseKeyDown = &UICombobox::OnMouseKeyDownMenu;
    _onMouseKeyUp = &UICombobox::OnMouseKeyUpMenu;
    _onMouseMove = &UICombobox::OnMouseMoveMenu;
    _updateRect = &UICombobox::UpdatedRectMenu;
    _menuDIV.Show ();

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StartWidgetCaptureMouse,
            ._params = this,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
