#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <pbr/utf8_parser.hpp>
#include <theme.hpp>
#include <ui_edit_box.hpp>


namespace editor {

namespace {

constexpr auto BLINK_PEDIOD = std::chrono::milliseconds ( 500U );

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
            ._backgroundColor = pbr::ColorValue ( 106U, 172U, 0U, 51U ),
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

    _valueDIV.AppendChildElement ( _cursorDIV );
    _valueDIV.AppendChildElement ( _selectionDIV );

    _textDIV.AppendChildElement ( _text );
    _valueDIV.AppendChildElement ( _textDIV );

    _columnDIV.AppendChildElement ( _valueDIV );

    _lineDIV.AppendChildElement ( _columnDIV );
    parent.AppendChildElement ( _lineDIV );

    if ( auto str = pbr::UTF8Parser::ToU32String ( value ); str ) [[likely]]
        _content = std::move ( *str );

    // FUCK
    // SwitchToNormalState ();
    _cursor = 2;
    _selection = 1;

    pbr::CSSComputedValues const &css = _textDIV.GetCSS ();

    fontStorage.GetStringMetrics ( _metrics,
        css._fontFile,
        static_cast<uint32_t> ( css._fontSize.GetValue () * pbr::CSSUnitToDevicePixel::GetInstance ()._fromPX ),
        _content
    );

    UpdateCursor ();
    SwitchToEditState ();
}

void UIEditBox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    ( this->*_onMouseMove ) ( event );
}

void UIEditBox::UpdatedRect () noexcept
{
    ( this->*_updateRect ) ();
}

void UIEditBox::OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept
{
    GX_DISABLE_WARNING ( 4061 )

    switch ( key )
    {
        case eKey::KeyLeft:
            MoveCursor ( -1, modifier );
        break;

        case eKey::KeyRight:
            MoveCursor ( 1, modifier );
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

void UIEditBox::OnMouseMoveEdit ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );
}

void UIEditBox::UpdatedRectEdit () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
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

void UIEditBox::MoveCursor ( int32_t offset, KeyModifier modifier ) noexcept
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

    int32_t const rangeCases[ 2U ][ 2U ] = { { _cursor, _selection }, { _selection, _cursor } };
    auto const &[ from, to ] = rangeCases[ static_cast<size_t> ( _cursor > _selection ) ];
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

void UIEditBox::SwitchToEditState () noexcept
{
    _text.SetColor ( theme::PRESS_COLOR );
    _cursorDIV.Show ();
    _selectionDIV.Show ();
    _onMouseMove = &UIEditBox::OnMouseMoveEdit;
    _updateRect = &UIEditBox::UpdatedRectEdit;

    ResetBlinkTimer ();

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StartWidgetCaptureInput,
            ._params = this,
            ._serialNumber = 0U
        }
    );
}

void UIEditBox::SwitchToNormalState () noexcept
{
    _blink.reset ();

    _text.SetColor ( theme::MAIN_COLOR );
    _cursorDIV.Hide ();
    _selectionDIV.Hide ();
    _onMouseMove = &UIEditBox::OnMouseMoveNormal;
    _updateRect = &UIEditBox::UpdatedRectNormal;
}

void UIEditBox::UpdateCursor () noexcept
{
    auto const c = _metrics[ static_cast<size_t> ( _cursor ) ];
    auto const s = _metrics[ static_cast<size_t> ( _selection ) ];

    float const cases[ 2U ][ 2U ] = { { c, s }, { s, c } };
    auto const &[ from, to ] = cases[ static_cast<size_t> ( _cursor > _selection ) ];

    GXVec3 offsets ( from, to - from, c );
    offsets.Multiply ( offsets, pbr::CSSUnitToDevicePixel::GetInstance ()._devicePXtoCSSPX );

    pbr::CSSComputedValues &selectionCSS = _selectionDIV.GetCSS ();
    selectionCSS._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, offsets._data[ 0U ] );
    selectionCSS._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, offsets._data[ 1U ] );
    _selectionDIV.Update ();

    _cursorDIV.GetCSS ()._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, offsets._data[ 2U ] );
    _cursorDIV.Update ();
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
