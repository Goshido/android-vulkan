#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/attribute_parser.hpp>
#include <pbr/ascii_string.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

namespace {

constexpr char32_t APOSTROPHE = U'\'';
constexpr char32_t SOLIDUS = U'/';
constexpr char32_t EQUALS_SIGN = U'=';
constexpr char32_t GRAVE_ACCENT = U'`';
constexpr char32_t GREATER_THAN_SIGN = U'>';
constexpr char32_t LESS_THAN_SIGN = U'<';
constexpr char32_t QUOTATION_MARK = U'"';

} // end of anonymous namespace

std::unordered_map<std::string_view, eAttribute> const AttributeParser::_attributes =
{
    { "class", eAttribute::Class },
    { "href", eAttribute::HREF },
    { "id", eAttribute::ID },
    { "rel", eAttribute::REL },
    { "src", eAttribute::SRC }
};

std::unordered_map<eAttribute, std::string_view> const AttributeParser::_names =
{
    { eAttribute::Class, "class" },
    { eAttribute::HREF, "href" },
    { eAttribute::ID, "id" },
    { eAttribute::No_Attribute, "no-attribute" },
    { eAttribute::REL, "rel" },
    { eAttribute::SRC, "src" }
};

std::optional<AttributeParser::Result> AttributeParser::Parse ( char const* html, Stream stream ) noexcept
{
    if ( !stream.ExpectNotEmpty ( html, "pbr::AttributeParser::Parse" ) )
        return std::nullopt;

    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    char32_t c = probe->_character;

    if ( ( c == SOLIDUS ) | ( c == GREATER_THAN_SIGN ) )
    {
        return Result
        {
            ._attribute = eAttribute::No_Attribute,
            ._newStream = stream,
            ._value = {}
        };
    }

    auto s = ASCIIString::Parse ( html, stream, ASCIIString::eParseMode::Letters );

    if ( !s )
        return std::nullopt;

    std::string_view name = s->_target;
    ASCIIString::ToLower ( name );
    auto const findResult = _attributes.find ( name );

    if ( findResult == _attributes.cend () )
    {
        android_vulkan::LogError ( "pbr::AttributeParser::Parse - %s:%zu: Unknown attribute '%s'.",
            html,
            stream._line,
            std::string ( name ).c_str ()
        );

        return std::nullopt;
    }

    stream = s->_newStream;

    if ( !stream.ExpectNotEmpty ( html, "pbr::AttributeParser::Parse" ) )
        return std::nullopt;

    auto skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;

    if ( !stream.ExpectNotEmpty ( html, "pbr::AttributeParser::Parse" ) )
        return std::nullopt;

    probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    stream = probe->_newStream;

    if ( probe->_character != EQUALS_SIGN )
    {
        return Result
        {
            ._attribute = findResult->second,
            ._newStream = stream,
            ._value = {}
        };
    }

    if ( !stream.ExpectNotEmpty ( html, "pbr::AttributeParser::Parse" ) )
        return std::nullopt;

    skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;

    if ( !stream.ExpectNotEmpty ( html, "pbr::AttributeParser::Parse" ) )
        return std::nullopt;

    probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    stream = probe->_newStream;

    constexpr auto checkApostrophe = [] ( char32_t c ) noexcept -> bool {
        return c != APOSTROPHE;
    };

    constexpr auto checkQuotationMark = [] ( char32_t c ) noexcept -> bool {
        return c != QUOTATION_MARK;
    };

    constexpr auto checkUnquoted = [] ( char32_t c ) noexcept -> bool {
        bool const c1 = !Whitespace::IsWhitespace ( c );
        auto const c2 = static_cast<bool> ( c != QUOTATION_MARK );
        auto const c3 = static_cast<bool> ( c != APOSTROPHE );
        auto const c4 = static_cast<bool> ( c != EQUALS_SIGN );
        auto const c5 = static_cast<bool> ( c != LESS_THAN_SIGN );
        auto const c6 = static_cast<bool> ( c != GREATER_THAN_SIGN );
        auto const c7 = static_cast<bool> ( c != GRAVE_ACCENT );
        return c1 & c2 & c3 & c4 & c5 & c6 & c7;
    };

    using Checker = bool ( * ) ( char32_t c ) noexcept;
    Checker checker;
    size_t quoteCorrector = 1U;

    c = probe->_character;
    std::u32string value {};

    switch ( c )
    {
        case APOSTROPHE:
            checker = checkApostrophe;
        break;

        case QUOTATION_MARK:
            checker = checkQuotationMark;
        break;

        default:
            if ( !checkUnquoted ( c ) )
            {
                android_vulkan::LogError ( "pbr::AttributeParser::Parse - %s:%zu: Empty value detected.",
                    html,
                    stream._line
                );

                return std::nullopt;
            }

            value.push_back ( c );
            checker = checkUnquoted;
            quoteCorrector = 0U;
        break;
    }

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( html, "pbr::AttributeParser::Parse" ) )
            return std::nullopt;

        probe = UTF8Parser::Parse ( html, stream );

        if ( !probe )
            return std::nullopt;

        c = probe->_character;

        if ( !checker ( c ) )
            break;

        stream = probe->_newStream;
        value.push_back ( c );
    }

    return Result
    {
        ._attribute = findResult->second,
        ._newStream = Stream ( stream._data.subspan ( quoteCorrector ), stream._line ),
        ._value = std::move ( value )
    };
}

char const* AttributeParser::ToString ( eAttribute attribute ) noexcept
{
    return _names.find ( attribute )->second.data ();
}

std::optional<std::unordered_set<std::u32string>> AttributeParser::Tokenize ( char const* html,
    size_t line,
    char const* entity,
    std::u32string const &value
) noexcept
{
    std::unordered_set<std::u32string> result {};

    size_t start = std::numeric_limits<size_t>::max ();
    bool hasStart = false;
    size_t const size = value.size ();

    auto const append = [ html, line, entity, &result ] ( std::u32string && token ) noexcept -> bool {
        if ( result.count ( token ) < 1U )
        {
            result.insert ( std::move ( token ) );
            return true;
        }

        auto const s = UTF8Parser::ToUTF8 ( token );

        if ( !s )
        {
            android_vulkan::LogError ( "pbr::AttributeParser::Tokenize - %s:%zu: Token parse error.", html, line );
            return false;
        }

        android_vulkan::LogError ( "pbr::AttributeParser::Tokenize - %s:%zu: Token duplication '%s' is detected "
            "in attribute '%s'.",
            html,
            line,
            s->c_str (),
            entity
        );

        return false;
    };

    for ( size_t i = 0U; i < size; ++i )
    {
        char32_t const &c = value[ i ];
        bool const isWhitespace = Whitespace::IsWhitespace ( c );

        if ( isWhitespace & !hasStart )
            continue;

        if ( !hasStart )
        {
            hasStart = true;
            start = i;
            continue;
        }

        if ( !isWhitespace )
            continue;

        if ( !append ( value.substr ( start, i - start ) ) )
            return std::nullopt;

        hasStart = false;
    }

    if ( hasStart && !append ( value.substr ( start ) ) )
        return std::nullopt;

    return result;
}

} // namespace pbr
