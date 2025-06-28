#include <precompiled_headers.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <pbr/color_property.hpp>
#include <pbr/div_html5_element.hpp>
#include <pbr/doctype_parser.hpp>
#include <pbr/font_family_property.hpp>
#include <pbr/html5_parser.hpp>
#include <pbr/img_html5_element.hpp>
#include <pbr/length_property.hpp>
#include <pbr/link_html5_element.hpp>
#include <pbr/set_attribute_checker.hpp>
#include <pbr/tag_parser.hpp>
#include <pbr/text_html5_element.hpp>
#include <pbr/unique_attribute_checker.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

CSSComputedValues &HTML5Parser::GetBodyCSS () noexcept
{
    return _bodyCSS;
}

std::u32string &HTML5Parser::GetBodyID () noexcept
{
    return _bodyID;
}

HTML5Children &HTML5Parser::GetBodyChildren () noexcept
{
    return _bodyChildren;
}

CSSParser &HTML5Parser::GetCSSParser () noexcept
{
    return _css;
}

bool HTML5Parser::Parse ( char const* html, Stream stream, char const* assetRoot ) noexcept
{
    auto skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return false;

    stream = *skip;

    if ( !stream.ExpectNotEmpty ( html, "pbr::HTML5Parser::Parse" ) )
        return false;

    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return false;

    if ( probe->_character != U'<' )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::Parse - %s:%zu: Expected '<' character (branch 1).",
            html,
            stream._line
        );

        return false;
    }

    stream = probe->_newStream;
    auto tag = TagParser::Parse ( html, stream );

    if ( !tag )
        return false;

    stream = tag->_newStream;

    if ( tag->_tag != HTML5Tag::eTag::Doctype )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::Parse - %s:%zu: Expected 'DOCTYPE' element.",
            html,
            stream._line
        );

        return false;
    }

    auto const dt = DoctypeParser::Parse ( html, stream );

    if ( !dt )
        return false;

    stream = *dt;

    skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return false;

    stream = *skip;
    probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return false;

    if ( probe->_character != U'<' )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::Parse - %s:%zu: Expected '<' character (branch 2).",
            html,
            stream._line
        );

        return false;
    }

    stream = probe->_newStream;
    tag = TagParser::Parse ( html, stream );

    if ( !tag )
        return false;

    stream = tag->_newStream;

    if ( tag->_tag == HTML5Tag::eTag::HTML )
        return ParseHTMLElement ( html, stream, assetRoot );

    android_vulkan::LogError ( "pbr::HTML5Parser::Parse - %s:%zu: Expected start tag of 'html' element.",
        html,
        stream._line
    );

    return false;
}

ParseResult HTML5Parser::ParseBodyElement ( char const* html, Stream stream, char const* assetRoot ) noexcept
{
    size_t const l = stream._line;

    if ( _bodyElementParsed )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseBodyElement - %s:%zu: Unexpected 'body' element. "
            "Element already present.",
            html,
            l
        );

        return std::nullopt;
    }

    std::unordered_set<std::u32string> idRegistry {};
    UniqueAttributeChecker idChecker ( html, eAttribute::ID, _bodyID, idRegistry );
    SetAttributeChecker classChecker ( html, eAttribute::Class, _bodyClasses );

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( html, "pbr::HTML5Parser::ParseBodyElement" ) )
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

        android_vulkan::LogError ( "pbr::HTML5Parser::ParseBodyElement - %s:%zu: Unsupported attribute detected '%s' "
            "for 'body' element",
            html,
            stream._line,
            AttributeParser::ToString ( a )
        );

        return std::nullopt;
    }

    if ( !stream.ExpectNotEmpty ( html, "pbr::HTML5Parser::ParseBodyElement" ) )
        return std::nullopt;

    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character == U'/' )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseBodyElement - %s:%zu: 'body' element without end tag "
            "detected.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    stream = probe->_newStream;

    if ( !stream.ExpectNotEmpty ( html, "pbr::HTML5Parser::ParseBodyElement" ) )
        return std::nullopt;

    for ( ; ; )
    {
        auto const skip = Whitespace::Skip ( html, stream );

        if ( !skip )
            return std::nullopt;

        stream = *skip;

        if ( !stream.ExpectNotEmpty ( html, "pbr::HTML5Parser::ParseBodyElement" ) )
            return std::nullopt;

        auto tag = TagParser::IsEndTag ( html, stream );

        if ( tag && tag->_tag == HTML5Tag::eTag::Body )
        {
            _bodyElementParsed = true;
            return tag->_newStream;
        }

        probe = UTF8Parser::Parse ( html, stream );

        if ( !probe )
            return std::nullopt;

        if ( probe->_character != U'<' )
        {
            auto const text = TextHTML5Element::Parse ( html, stream );

            if ( !text )
                return std::nullopt;

            _bodyChildren.push_back ( text->_element );
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

            _bodyChildren.push_back ( div->_element );
            stream = div->_newStream;
            continue;
        }

        if ( t == HTML5Tag::eTag::IMG )
        {
            auto const img = IMGHTML5Element::Parse ( html, stream, assetRoot, idRegistry );

            if ( !img )
                return std::nullopt;

            _bodyChildren.push_back ( img->_element );
            stream = img->_newStream;
            continue;
        }

        android_vulkan::LogError ( "pbr::HTML5Parser::ParseBodyElement - %s:%zu: Unexpected '%s' element",
            html,
            stream._line,
            t.ToString ()
        );

        return std::nullopt;
    }
}

bool HTML5Parser::ParseHTMLElement ( char const* html, Stream stream, char const* assetRoot ) noexcept
{
    auto skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return false;

    stream = *skip;
    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return false;

    if ( probe->_character != U'>' )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHTMLElement - %s:%zu: Expected '>'.", html, stream._line );
        return false;
    }

    stream = probe->_newStream;

    for ( ; ; )
    {
        skip = Whitespace::Skip ( html, stream );

        if ( !skip )
            return false;

        stream = *skip;
        auto tag = TagParser::IsEndTag ( html, stream );

        if ( tag )
        {
            if ( tag->_tag == HTML5Tag::eTag::HTML )
            {
                stream = tag->_newStream;
                break;
            }

            android_vulkan::LogError ( "pbr::HTML5Parser::ParseHTMLElement - %s:%zu: Expected end tag of "
                "'html' element.",
                html,
                stream._line
            );

            return false;
        }

        probe = UTF8Parser::Parse ( html, stream );

        if ( !probe )
            return false;

        if ( probe->_character != U'<' )
        {
            android_vulkan::LogError ( "pbr::HTML5Parser::ParseHTMLElement - %s:%zu: Unexpected end of the document.",
                html,
                stream._line
            );

            return false;
        }

        stream = probe->_newStream;
        tag = TagParser::Parse ( html, stream );

        if ( !tag )
            return false;

        HTML5Tag const t = tag->_tag;
        stream = tag->_newStream;
        bool isParsed = false;

        if ( t == HTML5Tag::eTag::Head )
        {
            auto const head = ParseHeadElement ( html, stream, assetRoot );

            if ( !head )
                return false;

            stream = *head;
            isParsed = true;
        }

        if ( t == HTML5Tag::eTag::Body )
        {
            auto const body = ParseBodyElement ( html, stream, assetRoot );

            if ( !body )
                return false;

            stream = *body;
            isParsed = true;
        }

        if ( isParsed )
            continue;

        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHTMLElement - %s:%zu: Expected 'head' or 'body' element.",
            html,
            stream._line
        );

        return false;
    }

    if ( !_bodyElementParsed )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHTMLElement - 'body' element is missing." );
        return false;
    }

    if ( !_bodyCSS.ApplyCSS ( html, _css, _bodyClasses, _bodyID ) )
        return false;

    for ( auto &element : _bodyChildren )
    {
        if ( !element->ApplyCSS ( html, _css ) )
        {
            return false;
        }
    }

    return true;
}

ParseResult HTML5Parser::ParseHeadElement ( char const* html, Stream stream, char const* assetRoot ) noexcept
{
    if ( _bodyElementParsed )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHeadElement - %s:%zu: Unexpected 'head' element. "
            "'body' element already present.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    if ( _headElementParsed )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHeadElement - %s:%zu: Unexpected 'head' element. "
            "Element already present.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    if ( !stream.ExpectNotEmpty ( html, "pbr::HTML5Parser::ParseHeadElement" ) )
        return std::nullopt;

    auto skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;
    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character != U'>' )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHeadElement - %s:%zu: Expected end of starting tag of "
            "'head' element.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    stream = probe->_newStream;

    skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    if ( !stream.ExpectNotEmpty ( html, "pbr::HTML5Parser::ParseHeadElement" ) )
        return std::nullopt;

    stream = *skip;
    probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character != U'<' )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHeadElement - %s:%zu: Unexpected end of the document.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    stream = probe->_newStream;
    auto tag = TagParser::Parse ( html, stream );

    if ( tag->_tag != HTML5Tag::eTag::Link )
    {
        android_vulkan::LogError ( "pbr::HTML5Parser::ParseHeadElement - %s:%zu: Only 'link' element supported.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    stream = tag->_newStream;
    auto link = LinkHTML5Element::Parse ( html, stream, assetRoot );

    if ( !link )
        return std::nullopt;

    char const* f = link->_cssFile.c_str ();

    android_vulkan::File file ( f );
    [[maybe_unused]] bool const alwaysTrue = file.LoadContent ();
    std::vector<uint8_t> &data = file.GetContent ();

    if ( !_css.Parse ( f, Stream ( Stream::Data ( data.data (), data.size () ), 1U ), std::move ( link->_assetRoot ) ) )
        return std::nullopt;

    stream = link->_newStream;
    skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;

    if ( tag = TagParser::IsEndTag ( html, stream ); tag && tag->_tag == HTML5Tag::eTag::Head )
    {
        _headElementParsed = true;
        return tag->_newStream;
    }

    android_vulkan::LogError ( "pbr::HTML5Parser::ParseHeadElement - %s:%zu: Expected end tag of 'head' element.",
        html,
        stream._line
    );

    return std::nullopt;
}

} // namespace pbr
