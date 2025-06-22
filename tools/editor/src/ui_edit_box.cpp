#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <pbr/utf8_parser.hpp>
#include <set_text_event.hpp>
#include <theme.hpp>
#include <ui_edit_box.hpp>


namespace editor {

namespace {

constexpr auto BLINK_PEDIOD = std::chrono::milliseconds ( 500U );

constexpr char32_t CTRL_A = 0x01;
constexpr char32_t CTRL_C = 0x03;
constexpr char32_t CTRL_V = 0x16;
constexpr char32_t CTRL_X = 0x18;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UIEditBox::UIEditBox ( MessageQueue &messageQueue,
    DIVUIElement &parent,
    pbr::FontStorage &fontStorage,
    std::string_view caption,
    std::string_view value,
    std::string &&name
) noexcept:
    Widget ( messageQueue ),
    _committed ( value ),
    _fontStorage ( fontStorage ),

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
            ._right = theme::ZERO_LENGTH,
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
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TEXT_COLOR_NORMAL,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile { theme::NORMAL_FONT_FAMILY },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
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
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile { theme::NORMAL_FONT_FAMILY },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 0.0F ),
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 65.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (value)"
    ),

    _cursorDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = theme::MAIN_COLOR,
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
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (cursor)"
    ),

    _selectionDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = pbr::ColorValue ( 255U, 255U, 255U, 26U ),
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
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (selection)"
    ),

    _textDIV ( messageQueue,
        _valueDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::AUTO_LENGTH,
            ._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._color = theme::MAIN_COLOR,
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
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (text)"
    ),

    _text ( messageQueue, _textDIV, value, name + " (text)" )
{
    _captionDIV.AppendChildElement ( _captionText );
    _columnDIV.AppendChildElement ( _captionDIV );

    _textDIV.AppendChildElement ( _text );
    _valueDIV.AppendChildElement ( _textDIV );

    _valueDIV.AppendChildElement ( _selectionDIV );
    _valueDIV.AppendChildElement ( _cursorDIV );

    _columnDIV.AppendChildElement ( _valueDIV );

    _lineDIV.AppendChildElement ( _columnDIV );
    parent.AppendChildElement ( _lineDIV );

    if ( auto str = pbr::UTF8Parser::ToU32String ( value ); str ) [[likely]]
        _content = std::move ( *str );

    UpdateMetrics ();
    SwitchToNormalState ();
}

void UIEditBox::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    ( this->*_onMouseKeyDown ) ( event );
}

void UIEditBox::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    ( this->*_onMouseKeyUp ) ( event );
}

void UIEditBox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    ( this->*_onMouseMove ) ( event );
}

void UIEditBox::UpdatedRect () noexcept
{
    ( this->*_updateRect ) ();
}

void UIEditBox::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

void UIEditBox::ApplyClipboard ( std::u32string const &text ) noexcept
{
    std::ignore = RemoveSelectedContent ();

    _content.insert ( static_cast<size_t> ( _cursor ), text );
    _cursor += static_cast<int32_t> ( text.size () );
    _text.SetText ( _content );

    _cursorDIV.Show ();
    _selectionDIV.Hide ();

    _selection = _cursor;

    UpdateMetrics ();
    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept
{
    GX_DISABLE_WARNING ( 4061 )

    switch ( key )
    {
        case eKey::KeyBackspace:
            Erase ( -1 );
        break;

        case eKey::KeyDel:
            Erase ( 0 );
        break;

        case eKey::KeyEnd:
            MoveCursor ( static_cast<int32_t> ( _content.size () ), !modifier.AnyShiftPressed () );
        break;

        case eKey::KeyEnter:
            Commit ();
        break;

        case eKey::KeyHome:
            MoveCursor ( 0, !modifier.AnyShiftPressed () );
        break;

        case eKey::KeyLeft:
            OffsetCursor ( -1, modifier );
        break;

        case eKey::KeyRight:
            OffsetCursor ( 1, modifier );
        break;

        default:
            // NOTHING
        break;
    }

    GX_ENABLE_WARNING ( 4061 )
}

void UIEditBox::OnMouseLeave () noexcept
{
    _text.SetColor ( theme::MAIN_COLOR );
}

void UIEditBox::OnTyping ( char32_t character ) noexcept
{
    if ( character >= U' ' ) [[likely]]
    {
        Append ( character );
        return;
    }

    switch ( character )
    {
        case CTRL_A:
            SelectAll ();
        break;

        case CTRL_C:
            Copy ();
        break;

        case CTRL_V:
            Paste ();
        break;

        case CTRL_X:
            Cut ();
        break;

        default:
            // NOTHING
        break;
    }
}

void UIEditBox::OnMouseButtonDownEdit ( MouseButtonEvent const &event ) noexcept
{
    if ( !_rect.IsOverlapped ( event._x, event._y ) )
    {
        SwitchToNormalState ();

        _messageQueue.EnqueueBack (
            {
                ._type = eMessageType::MouseMoved,

                ._params = new MouseMoveEvent
                {
                    ._x = event._x,
                    ._y = event._y,
                    ._eventID = _eventID - 1U
                },

                ._serialNumber = 0U
            }
        );

        _messageQueue.EnqueueBack (
            {
                ._type = eMessageType::MouseButtonDown,
                ._params = new MouseButtonEvent ( event ),
                ._serialNumber = 0U
            }
        );

        return;
    }

    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    CaptureMouse ();

    _leftMouseButtonPressed = true;
    _cursor = FindClosestSymbol ( event._x );

    int32_t const cases[] = { _cursor, _selection };
    _selection = cases[ static_cast<size_t> ( event._modifier.AnyShiftPressed () ) ];

    _cursorDIV.Show ();

    if ( _cursor == _selection )
        _selectionDIV.Hide ();
    else
        _selectionDIV.Show ();

    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::OnMouseButtonUpEdit ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key == eKey::LeftMouseButton ) [[likely]]
    {
        ReleaseMouse ();
        _leftMouseButtonPressed = false;
    }
}

void UIEditBox::OnMouseMoveEdit ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( !_leftMouseButtonPressed ) [[likely]]
        return;

    _cursor = FindClosestSymbol ( event._x );
    _cursorDIV.Show ();

    if ( _cursor == _selection )
        _selectionDIV.Hide ();
    else
        _selectionDIV.Show ();

    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::UpdatedRectEdit () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
}

void UIEditBox::OnMouseButtonDownNormal ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _cursor = FindClosestSymbol ( event._x );
    _selection = _cursor;
    _leftMouseButtonPressed = true;
    SwitchToEditState ();
}

void UIEditBox::OnMouseButtonUpNormal ( MouseButtonEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

void UIEditBox::OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ChangeCursor,
            ._params = reinterpret_cast<void*> ( eCursor::IBeam ),
            ._serialNumber = 0U
        }
    );

    _text.SetColor ( theme::HOVER_COLOR );
}

void UIEditBox::UpdatedRectNormal () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
}

void UIEditBox::Append ( char32_t character ) noexcept
{
    std::ignore = RemoveSelectedContent ();

    _content.insert ( static_cast<size_t> ( _cursor ), 1U, character );
    _text.SetText ( _content );

    _cursorDIV.Show ();
    _selectionDIV.Hide ();

    ++_cursor;
    _selection = _cursor;

    UpdateMetrics ();
    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::Commit () noexcept
{
    if ( auto v = pbr::UTF8Parser::ToUTF8 ( _content ); *v != _committed ) [[likely]]
    {
        _committed = std::move ( *v );
        _callback ( _committed );
    }

    SwitchToNormalState ();
}

void UIEditBox::Copy () noexcept
{
    if ( _cursor == _selection ) [[unlikely]]
        return;

    auto const [from, to] = GetSelection ();
    auto const begin = _content.cbegin ();
    using Offset = decltype ( begin )::difference_type;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::WriteClipboard,
            ._params = new std::u32string ( begin + static_cast<Offset> ( from ), begin + static_cast<Offset> ( to ) ),
            ._serialNumber = 0U
        }
    );
}

void UIEditBox::Cut () noexcept
{
    Copy ();

    if ( !RemoveSelectedContent () ) [[unlikely]]
        return;

    _text.SetText ( _content );

    _cursorDIV.Show ();
    _selectionDIV.Hide ();

    _selection = _cursor;

    UpdateMetrics ();
    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::Erase ( int32_t offset ) noexcept
{
    if ( !RemoveSelectedContent () ) [[likely]]
    {
        int32_t const c = _cursor + offset;

        if ( ( c < 0 ) | ( c >= static_cast<int32_t> ( _content.size () ) ) ) [[unlikely]]
            return;

        _cursor = c;
        _content.erase ( static_cast<size_t> ( c ), 1U );
    }

    _text.SetText ( _content );

    _cursorDIV.Show ();
    _selectionDIV.Hide ();

    _selection = _cursor;

    UpdateMetrics ();
    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::Paste () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ReadClipboardRequest,
            ._params = this,
            ._serialNumber = 0U
        }
    );
}

int32_t UIEditBox::FindClosestSymbol ( int32_t x ) const noexcept
{
    auto const rawOffset = static_cast<float> ( x - _rect._left );
    float closest = std::numeric_limits<float>::max ();
    int32_t c = -1;

    for ( float const offset : _metrics )
    {
        if ( float const len = std::abs ( rawOffset - offset ); len > std::exchange ( closest, len ) ) [[unlikely]]
            break;

        ++c;
    }

    return std::clamp ( c, 0, static_cast<int32_t> ( _content.size () ) );
}

std::pair<int32_t, int32_t> UIEditBox::GetSelection () const noexcept
{
    int32_t const cases[ 2U ][ 2U ] = { { _cursor, _selection }, { _selection, _cursor } };
    auto const [from, to] = cases[ static_cast<size_t> ( _cursor > _selection ) ];
    return std::make_pair ( from, to );
}

void UIEditBox::ModifySelection ( int32_t offset, int32_t cursorLimit ) noexcept
{
    _cursor = std::clamp ( _cursor + offset, 0, cursorLimit );
    _cursorDIV.Show ();

    if ( _cursor == _selection ) [[unlikely]]
        _selectionDIV.Hide ();
    else
        _selectionDIV.Show ();

    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::MoveCursor ( int32_t cursor, bool cancelSelection ) noexcept
{
    _cursor = cursor;
    int32_t const cases[] = { _selection, cursor };
    _selection = cases[ static_cast<size_t> ( cancelSelection ) ];

    if ( _cursor == _selection )
        _selectionDIV.Hide ();
    else
        _selectionDIV.Show ();

    _cursorDIV.Show ();
    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::OffsetCursor ( int32_t offset, KeyModifier modifier ) noexcept
{
    bool const shift = modifier.AnyShiftPressed ();
    bool const ctrl = modifier.AnyCtrlPressed ();

    int32_t const cursorLimitCases[] = { static_cast<int32_t> ( _metrics.size () - 1U ), 0 };
    int32_t const cursorLimit = cursorLimitCases[ static_cast<size_t> ( _metrics.empty () ) ];

    if ( shift )
    {
        if ( ctrl )
        {
            JumpOverWord ( offset, false );
            return;
        }

        ModifySelection ( offset, cursorLimit );
        return;
    }

    if ( ctrl )
    {
        JumpOverWord ( offset, true );
        return;
    }

    auto const [from, to] = GetSelection ();
    int32_t const borderCases[] = { from, to };

    int32_t const cursorCases[] =
    {
        std::clamp ( _cursor + offset, 0, cursorLimit ),
        borderCases[ static_cast<size_t> ( offset > 0 ) ]
    };

    _cursor = cursorCases[ static_cast<size_t> ( _cursor != _selection ) ];
    _selection = _cursor;

    _cursorDIV.Show ();
    _selectionDIV.Hide ();
    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::JumpOverWord ( int32_t offset, bool cancelSelection ) noexcept
{
    auto const limit = static_cast<int32_t> ( _content.size () );
    _cursor = std::clamp ( offset < 0 ? JumpOverWordLeft ( limit ) : JumpOverWordRight ( limit ), 0, limit );
    int32_t const cases[] = { _selection, _cursor };
    _selection = cases[ static_cast<size_t> ( cancelSelection ) ];

    if ( _cursor == _selection )
        _selectionDIV.Hide ();
    else
        _selectionDIV.Show ();

    _cursorDIV.Show ();
    UpdateCursor ();
    ResetBlinkTimer ();
}

int32_t UIEditBox::JumpOverWordLeft ( int32_t limit ) const noexcept
{
    if ( limit < 1 ) [[unlikely]]
        return 0;

    char32_t const* const content = _content.data ();
    int32_t c = _cursor - 1;
    eLetterType type = ResolveLetterType ( content[ static_cast<size_t> ( c ) ] );

    while ( ( c >= 0 ) & ( type == eLetterType::Whitespace ) )
        type = ResolveLetterType ( content[ static_cast<size_t> ( --c ) ] );

    if ( type == eLetterType::Whitespace )
        return c;

    for ( --c; c >= 0; --c )
    {
        if ( type != ResolveLetterType ( content[ static_cast<size_t> ( c ) ] ) )
        {
            return ++c;
        }
    }

    return c;
}

int32_t UIEditBox::JumpOverWordRight ( int32_t limit ) const noexcept
{
    if ( limit < 1 ) [[unlikely]]
        return 0;

    char32_t const* const content = _content.data ();
    int32_t c = _cursor;
    eLetterType const current = ResolveLetterType ( content[ static_cast<size_t> ( c ) ] );
    eLetterType next = eLetterType::Whitespace;

    for ( ++c; c < limit; ++c )
    {
        if ( next = ResolveLetterType ( content[ static_cast<size_t> ( c ) ] ); next != current )
        {
            break;
        }
    }

    if ( next != eLetterType::Whitespace )
        return c;

    for ( ++c; c < limit; ++c )
    {
        if ( ResolveLetterType ( content[ static_cast<size_t> ( c ) ] ) != eLetterType::Whitespace )
        {
            break;
        }
    }

    return c;
}

bool UIEditBox::RemoveSelectedContent () noexcept
{
    if ( _cursor == _selection ) [[likely]]
        return false;

    auto const [from, to] = GetSelection ();
    _cursor = from;

    auto const begin = _content.cbegin ();
    using Offset = decltype ( begin )::difference_type;
    _content.erase ( begin + static_cast<Offset> ( from ), begin + static_cast<Offset> ( to ) );
    return true;
}

void UIEditBox::ResetBlinkTimer () noexcept
{
    _blink = std::make_unique<Timer> ( _messageQueue,
        Timer::eType::Repeat,
        BLINK_PEDIOD,

        [ this ] ( Timer::ElapsedTime &&/*elapsedTime*/ ) noexcept {
            if ( _cursorDIV.IsVisible () )
            {
                _cursorDIV.Hide ();
                return;
            }

            _cursorDIV.Show ();
        }
    );
}

void UIEditBox::SelectAll () noexcept
{
    _selection = 0;
    _cursor = static_cast<int32_t> ( _content.size () );

    _cursorDIV.Show ();
    _selectionDIV.Show ();

    UpdateCursor ();
    ResetBlinkTimer ();
}

void UIEditBox::SwitchToEditState () noexcept
{
    _text.SetColor ( theme::PRESS_COLOR );
    _cursorDIV.Show ();
    _selectionDIV.Show ();

    _onMouseKeyDown = &UIEditBox::OnMouseButtonDownEdit;
    _onMouseKeyUp = &UIEditBox::OnMouseButtonUpEdit;
    _onMouseMove = &UIEditBox::OnMouseMoveEdit;
    _updateRect = &UIEditBox::UpdatedRectEdit;

    UpdateCursor ();
    ResetBlinkTimer ();
    SetFocus ();
}

void UIEditBox::SwitchToNormalState () noexcept
{
    KillFocus ();
    _blink.reset ();

    _text.SetColor ( theme::MAIN_COLOR );
    _cursorDIV.Hide ();
    _selectionDIV.Hide ();

    _onMouseKeyDown = &UIEditBox::OnMouseButtonDownNormal;
    _onMouseKeyUp = &UIEditBox::OnMouseButtonUpNormal;
    _onMouseMove = &UIEditBox::OnMouseMoveNormal;
    _updateRect = &UIEditBox::UpdatedRectNormal;
}

void UIEditBox::UpdateCursor () noexcept
{
    GXVec3 offsets ( 0.0F, 0.0F, 0.0F );

    if ( !_content.empty () )
    {
        auto const c = _metrics[ static_cast<size_t> ( _cursor ) ];
        auto const s = _metrics[ static_cast<size_t> ( _selection ) ];

        float const cases[ 2U ][ 2U ] = { { c, s }, { s, c } };
        auto const [from, to] = cases[ static_cast<size_t> ( _cursor > _selection ) ];
        offsets.Multiply ( GXVec3 ( from, to - from, c ), pbr::CSSUnitToDevicePixel::GetInstance ()._devicePXtoCSSPX );
    }

    pbr::CSSComputedValues &selectionCSS = _selectionDIV.GetCSS ();
    selectionCSS._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, offsets._data[ 0U ] );
    selectionCSS._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, offsets._data[ 1U ] );
    _selectionDIV.Update ();

    _cursorDIV.GetCSS ()._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, offsets._data[ 2U ] );
    _cursorDIV.Update ();
}

void UIEditBox::UpdateMetrics () noexcept
{
    pbr::CSSComputedValues const &css = _textDIV.GetCSS ();

    _fontStorage.GetStringMetrics ( _metrics,
        css._fontFile,
        static_cast<uint32_t> ( css._fontSize.GetValue () * pbr::CSSUnitToDevicePixel::GetInstance ()._fromPX ),
        _content
    );
}

UIEditBox::eLetterType UIEditBox::ResolveLetterType ( char32_t c ) noexcept
{
    switch ( c )
    {
        case U' ':
        return eLetterType::Whitespace;

        case U'-':
            [[fallthrough]];
        case U',':
            [[fallthrough]];
        case U';':
            [[fallthrough]];
        case U':':
            [[fallthrough]];
        case U'!':
            [[fallthrough]];
        case U'?':
            [[fallthrough]];
        case U'.':
            [[fallthrough]];
        case U'\'':
            [[fallthrough]];
        case U'"':
            [[fallthrough]];
        case U'(':
            [[fallthrough]];
        case U')':
            [[fallthrough]];
        case U'[':
            [[fallthrough]];
        case U']':
            [[fallthrough]];
        case U'{':
            [[fallthrough]];
        case U'}':
            [[fallthrough]];
        case U'@':
            [[fallthrough]];
        case U'*':
            [[fallthrough]];
        case U'/':
            [[fallthrough]];
        case U'\\':
            [[fallthrough]];
        case U'&':
            [[fallthrough]];
        case U'#':
            [[fallthrough]];
        case U'%':
            [[fallthrough]];
        case U'^':
            [[fallthrough]];
        case U'+':
            [[fallthrough]];
        case U'<':
            [[fallthrough]];
        case U'=':
            [[fallthrough]];
        case U'>':
            [[fallthrough]];
        case U'|':
            [[fallthrough]];
        case U'~':
            [[fallthrough]];
        case U'$':
        return eLetterType::Punctuation;

        default:
        return eLetterType::AlphaNumeric;
    }
}

} // namespace editor
