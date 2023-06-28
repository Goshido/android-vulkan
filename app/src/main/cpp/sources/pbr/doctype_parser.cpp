#include <logger.h>
#include <pbr/doctype_parser.h>
#include <pbr/ascii_string.h>
#include <pbr/utf8_parser.h>
#include <pbr/whitespace.h>


namespace pbr {

ParseResult DoctypeParser::Parse ( char const* html, Stream stream ) noexcept
{
    if ( !stream.ExpectNotEmpty ( html, "pbr::DoctypeParser::Parse" ) )
        return std::nullopt;

    size_t const l = stream._line;
    auto skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    Stream s = *skip;

    if ( !s.ExpectNotEmpty ( html, "pbr::DoctypeParser::Parse" ) )
        return std::nullopt;

    auto const name = ASCIIString::Parse ( html, s, ASCIIString::eParseMode::Alphanumeric );

    if ( !name )
        return std::nullopt;

    s = name->_newStream;
    std::string_view const& n = name->_target;

    if ( n.empty () )
    {
        android_vulkan::LogError ( "pbr::DoctypeParser::Parse - %s:%zu: Missing 'html' attribute.", html, l );
        return std::nullopt;
    }

    if ( !s.ExpectNotEmpty ( html, "pbr::DoctypeParser::Parse" ) )
        return std::nullopt;

    ASCIIString::ToLower ( n );

    if ( n != "html" )
    {
        android_vulkan::LogError ( "pbr::DoctypeParser::Parse - %s:%zu: Expected 'html'. Got '%s'.",
            html,
            s._line,
            std::string ( n ).c_str ()
        );

        return std::nullopt;
    }

    skip = Whitespace::Skip ( html, s );

    if ( !skip )
        return std::nullopt;

    s = *skip;

    if ( !s.ExpectNotEmpty ( html, "pbr::DoctypeParser::Parse" ) )
        return std::nullopt;

    auto const probe = UTF8Parser::Parse ( html, s );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character != U'>' )
    {
        android_vulkan::LogError ( "pbr::DoctypeParser::Parse - %s:%zu: Expected end of '!DOCTYPE' element.",
            html,
            s._line
        );

        return std::nullopt;
    }

    return probe->_newStream;
}

} // namespace pbr
