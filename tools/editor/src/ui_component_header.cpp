#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <glyphs.hpp>
#include <property_tool.hpp>
#include <theme.hpp>
#include <ui_component_header.hpp>


namespace editor {

UIComponentHeader::UIComponentHeader ( DIVUIElement &parent,
    StringProperty &name,
    std::string_view glyph,
    bool &collapse
) noexcept:
    _root ( parent,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
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
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = theme::WHOLE_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 21.0F )
        },

        name + " (component header root)"
    ),

    _info ( _root,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
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
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = theme::WHOLE_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 18.0F )
        },

        name + " (component header info)"
    ),

    _glyph ( _info,
        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F ),
            ._right = theme::AUTO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { glyph::FONT_FAMILY },
            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::PX, 15.0F ),
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
            ._position = pbr::PositionProperty::eValue::Absolute,
            ._textAlign = pbr::TextAlignProperty::eValue::Center,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F )
        },

        glyph,
        name + " (component header glyph)"
    ),

    _name ( _info,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
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
            ._paddingLeft = pbr::LengthValue ( pbr::LengthValue::eType::PX, 25.0F ),
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = theme::WHOLE_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 10.0F )
        },

        name,
        name + " (component header name)"
    ),

    _expand ( _info,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 5.0F ),
            ._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
            ._color = theme::MAIN_COLOR,
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

            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent,
                100.0F,

                {
                    {
                        ._value = -8.0F,
                        ._type = pbr::LengthValue::eType::PX
                    }
                }
            ),

            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 14.0F )
        },

        collapse ? glyph::COMBOBOX_UP : glyph::COMBOBOX_DOWN,
        name + " (component header collapse button)"
    ),

    _underline ( _root,
        pbr::CSSComputedValues
        {
            ._backgroundColor = theme::MAIN_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = pbr::ColorValue ( 255U, 255U, 255U, 255U ),
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile{ theme::NORMAL_FONT_FAMILY },
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
            ._width = theme::WHOLE_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F )
        },

        name + " (component header underline)"
    ),

    _collapse ( collapse ),
    _property ( &name )
{
    _info.AppendChildElement ( _underline );

    _root.AppendChildElement ( _info );
    _root.AppendChildElement ( _underline );
    parent.AppendChildElement ( _root );

    _connection = name.Connect (
        [ &n = _name, &name ] () noexcept {
            n.SetText ( static_cast<std::string const &> ( name ) );
        }
    );
}

UIComponentHeader::~UIComponentHeader () noexcept
{
    if ( _property ) [[likely]]
    {
        _property->Disconnect ( _connection );
    }
}

void UIComponentHeader::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key == eKey::LeftMouseButton ) [[likely]]
    {
        SetColor ( theme::PRESS_COLOR );
    }
}

void UIComponentHeader::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _collapse = !_collapse;
    SetColor ( theme::HOVER_COLOR );
    _expand.SetText ( _collapse ? glyph::COMBOBOX_UP : glyph::COMBOBOX_DOWN );

    PropertyTool::Instance ().Rebuild ();
}

void UIComponentHeader::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return;

    ChangeCursor ( eCursor::Arrow );
    SetColor ( theme::HOVER_COLOR );
}

void UIComponentHeader::UpdatedRect () noexcept
{
    _rect.From ( _root.GetAbsoluteRect () );
}

void UIComponentHeader::Disconnect () noexcept
{
    std::exchange ( _property, nullptr )->Disconnect ( _connection );
}

void UIComponentHeader::DumpLayout ( pbr::SVGWriter &writer ) const noexcept
{
    _root.DumpLayout ( writer );
    _info.DumpLayout ( writer );
    _glyph.DumpLayout ( writer );
    _name.DumpLayout ( writer );
    _underline.DumpLayout ( writer );
    _expand.DumpLayout ( writer );
}

void UIComponentHeader::SetName ( std::string const &name ) noexcept
{
    _name.SetText ( name );
}

void UIComponentHeader::OnMouseLeave () noexcept
{
    SetColor ( theme::MAIN_COLOR );
}

void UIComponentHeader::SetColor ( pbr::ColorValue const &color ) noexcept
{
    _glyph.SetColor ( color );
    _name.SetColor ( color );
    _expand.SetColor ( color );

    _underline.GetCSS ()._backgroundColor = color;
    _underline.Update ();
}

} // namespace editor
