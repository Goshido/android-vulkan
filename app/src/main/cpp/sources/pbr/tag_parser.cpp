#include <logger.hpp>
#include <pbr/tag_parser.hpp>
#include <pbr/ascii_string.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cctype>
#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

std::optional<TagParser::Result> TagParser::IsEndTag ( char const* html, Stream stream ) noexcept
{
    constexpr uint8_t const prefix[] = { static_cast<uint8_t> ( '<' ), static_cast<uint8_t> ( '/' ) };

    constexpr size_t endTagPrefixCharacters = std::size ( prefix );
    constexpr size_t minTagNameNameCharacters = 1U;
    constexpr size_t endTagSuffixCharacters = 1U;
    constexpr size_t minCharacters = endTagPrefixCharacters + minTagNameNameCharacters + endTagSuffixCharacters;

    if ( stream._data.size () < minCharacters )
        return std::nullopt;

    if ( std::memcmp ( stream._data.data (), prefix, endTagPrefixCharacters ) != 0 )
        return std::nullopt;

    stream.Init ( stream._data.subspan ( endTagPrefixCharacters ), stream._line );
    auto const name = ASCIIString::Parse ( html, stream, ASCIIString::eParseMode::Alphanumeric );

    if ( !name )
        return std::nullopt;

    std::string_view const target = name->_target;
    auto const tag = HTML5Tag::Parse ( target );

    if ( !tag )
    {
        android_vulkan::LogError ( "pbr::TagParser::IsEndTag - %s:%zu: Unknown tag '%s'.",
            html,
            stream._line,
            std::string ( target ).c_str ()
        );

        return std::nullopt;
    }

    stream = name->_newStream;
    auto const skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;

    if ( !stream.ExpectNotEmpty ( html, "pbr::TagParser::IsEndTag" ) )
        return std::nullopt;

    auto const probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character != U'>' )
    {
        android_vulkan::LogError ( "pbr::TagParser::IsEndTag - %s:%zu: Expected '>' at end tag.", html, stream._line );
        return std::nullopt;
    }

    return Result
    {
        ._newStream = probe->_newStream,
        ._tag = *tag
    };
}

std::optional<TagParser::Result> TagParser::Parse ( char const* html, Stream stream ) noexcept
{
    if ( !stream.ExpectNotEmpty ( html, "pbr::TagParser::Parse" ) )
        return std::nullopt;

    size_t const line = stream._line;
    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character == U'<' )
    {
        stream = probe->_newStream;
        probe = UTF8Parser::Parse ( html, stream );

        if ( !probe )
        {
            return std::nullopt;
        }
    }

    if ( probe->_character == U'!' )
    {
        // !DOCTYPE handing.
        stream.Init ( stream._data.subspan ( 1U ), stream._line );
    }

    auto const name = ASCIIString::Parse ( html, stream, ASCIIString::eParseMode::Alphanumeric );

    if ( !name )
        return std::nullopt;

    std::string_view const target = name->_target;
    auto const tag = HTML5Tag::Parse ( target );

    if ( !tag )
    {
        android_vulkan::LogError ( "pbr::TagParser::Parse - %s:%zu: Unknown tag '%s'.",
            html,
            line,
            std::string ( target ).c_str ()
        );

        return std::nullopt;
    }

    return Result
    {
        ._newStream = name->_newStream,
        ._tag = *tag
    };
}

} // namespace pbr
