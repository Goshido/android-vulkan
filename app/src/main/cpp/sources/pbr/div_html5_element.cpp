#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/div_html5_element.hpp>
#include <pbr/img_html5_element.hpp>
#include <pbr/set_attribute_checker.hpp>
#include <pbr/tag_parser.hpp>
#include <pbr/text_html5_element.hpp>
#include <pbr/unique_attribute_checker.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

DIVHTML5Element::DIVHTML5Element ( std::u32string &&id,
    std::unordered_set<std::u32string> &&classes,
    HTML5Children &&children
) noexcept:
    HTML5Element ( HTML5Tag::eTag::DIV ),

    _cssComputedValues
    {
        ._backgroundColor = ColorValue ( 0U, 0U, 0U, 0U ),
        ._backgroundSize = LengthValue ( LengthValue::eType::Percent, 100.0F ),
        ._bottom = LengthValue ( LengthValue::eType::Auto, 0.0F ),
        ._left = LengthValue ( LengthValue::eType::Auto, 0.0F ),
        ._right = LengthValue ( LengthValue::eType::Auto, 0.0F ),
        ._top = LengthValue ( LengthValue::eType::Auto, 0.0F ),
        ._color {},
        ._display = DisplayProperty::eValue::Block,
        ._fontFile {},
        ._fontSize = LengthValue ( LengthValue::eType::EM, 1.0F ),
        ._lineHeight = LengthValue ( LengthValue::eType::Inherit, 42.0F ),
        ._marginBottom = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._marginLeft = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._marginRight = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._marginTop = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._paddingBottom = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._paddingLeft = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._paddingRight = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._paddingTop = LengthValue ( LengthValue::eType::PX, 0.0F ),
        ._position = PositionProperty::eValue::Static,
        ._textAlign = TextAlignProperty::eValue::Inherit,
        ._verticalAlign = VerticalAlignProperty::eValue::Inherit,
        ._width = LengthValue ( LengthValue::eType::Auto, 0.0F ),
        ._height = LengthValue ( LengthValue::eType::Auto, 0.0F )
    },

    _children ( std::move ( children ) ),
    _classes ( std::move ( classes ) ),
    _id ( std::move ( id ) )
{
    // NOTHING
}

HTML5Children &DIVHTML5Element::GetChildren () noexcept
{
    return _children;
}

std::u32string &DIVHTML5Element::GetID () noexcept
{
    return _id;
}

// NOLINTNEXTLINE - recursive call.
std::optional<HTML5Element::Result> DIVHTML5Element::Parse ( char const* html,
    Stream stream,
    char const* assetRoot,
    std::unordered_set<std::u32string> &idRegistry
) noexcept
{
    std::u32string id {};
    UniqueAttributeChecker idChecker ( html, eAttribute::ID, id, idRegistry );

    std::unordered_set<std::u32string> classes {};
    SetAttributeChecker classChecker ( html, eAttribute::Class, classes );

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( html, "pbr::DIVHTML5Element::Parse" ) )
            return std::nullopt;

        auto const skip = Whitespace::Skip ( html, stream );

        if ( !skip )
            return std::nullopt;

        stream = *skip;
        auto att = AttributeParser::Parse ( html, stream );

        if ( !att )
            return std::nullopt;

        eAttribute const a = att->_attribute;

        if ( a == eAttribute::No_Attribute )
            break;

        auto action = idChecker.Process ( *att );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = att->_newStream;
            continue;
        }

        action = classChecker.Process ( *att );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = att->_newStream;
            continue;
        }

        android_vulkan::LogError ( "pbr::DIVHTML5Element::Parse - %s:%zu: Unsupported attribute detected '%s' for "
            "'div' element.",
            html,
            stream._line,
            AttributeParser::ToString ( a )
        );

        return std::nullopt;
    }

    if ( !stream.ExpectNotEmpty ( html, "pbr::DIVHTML5Element::Parse" ) )
        return std::nullopt;

    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character == U'/' )
    {
        android_vulkan::LogError ( "pbr::DIVHTML5Element::Parse - %s:%zu: 'div' element without end tag detected.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    stream = probe->_newStream;

    if ( !stream.ExpectNotEmpty ( html, "pbr::DIVHTML5Element::Parse" ) )
        return std::nullopt;

    HTML5Children children {};

    for ( ; ; )
    {
        auto const skip = Whitespace::Skip ( html, stream );

        if ( !skip )
            return std::nullopt;

        stream = *skip;

        if ( !stream.ExpectNotEmpty ( html, "pbr::DIVHTML5Element::Parse" ) )
            return std::nullopt;

        auto tag = TagParser::IsEndTag ( html, stream );

        if ( tag && tag->_tag == HTML5Tag::eTag::DIV )
        {
            return Result
            {
                ._element = std::make_shared<DIVHTML5Element> ( std::move ( id ),
                    std::move ( classes ),
                    std::move ( children )
                ),

                ._newStream = tag->_newStream
            };
        }

        probe = UTF8Parser::Parse ( html, stream );

        if ( !probe )
            return std::nullopt;

        if ( probe->_character != U'<' )
        {
            auto const text = TextHTML5Element::Parse ( html, stream );

            if ( !text )
                return std::nullopt;

            children.push_back ( text->_element );
            stream = text->_newStream;
            continue;
        }

        stream = probe->_newStream;
        tag = TagParser::Parse ( html, stream );

        if ( !tag )
            return std::nullopt;

        stream = tag->_newStream;
        HTML5Tag const t = tag->_tag;

        if ( t == HTML5Tag::eTag::DIV )
        {
            auto const div = DIVHTML5Element::Parse ( html, stream, assetRoot, idRegistry );

            if ( !div )
                return std::nullopt;

            children.push_back ( div->_element );
            stream = div->_newStream;
            continue;
        }

        if ( t == HTML5Tag::eTag::IMG )
        {
            auto const img = IMGHTML5Element::Parse ( html, stream, assetRoot, idRegistry );

            if ( !img )
                return std::nullopt;

            children.push_back ( img->_element );
            stream = img->_newStream;
            continue;
        }

        android_vulkan::LogError ( "pbr::DIVHTML5Element::Parse - %s:%zu: Unexpected '%s' element",
            html,
            stream._line,
            t.ToString ()
        );

        return std::nullopt;
    }
}

bool DIVHTML5Element::ApplyCSS ( char const* html, CSSParser const &css ) noexcept
{
    if ( !_cssComputedValues.ApplyCSS ( html, css, _classes, _id ) )
        return false;

    for ( auto &element : _children )
    {
        if ( !element->ApplyCSS ( html, css ) )
        {
            return false;
        }
    }

    return true;
}

} // namespace pbr
