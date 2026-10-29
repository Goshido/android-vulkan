#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/ascii_string.hpp>
#include <pbr/property_parser.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

namespace {

std::unordered_map<std::string_view, Property::eType> const g_properties =
{
    { "background-color", Property::eType::BackgroundColor },
    { "background-size", Property::eType::BackgroundSize },
    { "bottom", Property::eType::Bottom },
    { "color", Property::eType::Color },
    { "display", Property::eType::Display },
    { "font-family", Property::eType::FontFamily },
    { "font-size", Property::eType::FontSize },
    { "height", Property::eType::Height },
    { "left", Property::eType::Left },
    { "line-height", Property::eType::LineHeight },
    { "margin", Property::eType::Margin },
    { "margin-bottom", Property::eType::MarginBottom },
    { "margin-left", Property::eType::MarginLeft },
    { "margin-right", Property::eType::MarginRight },
    { "margin-top", Property::eType::MarginTop },
    { "padding", Property::eType::Padding },
    { "padding-bottom", Property::eType::PaddingBottom },
    { "padding-left", Property::eType::PaddingLeft },
    { "padding-right", Property::eType::PaddingRight },
    { "padding-top", Property::eType::PaddingTop },
    { "position", Property::eType::Position },
    { "right", Property::eType::Right },
    { "src", Property::eType::SRC },
    { "text-align", Property::eType::TextAlign },
    { "top", Property::eType::Top },
    { "vertical-align", Property::eType::VerticalAlign },
    { "width", Property::eType::Width }
};

constexpr char32_t LEFT_PARENTHESIS = U'(';
constexpr char32_t RIGHT_PARENTHESIS = U')';
constexpr char32_t MINUS = U'-';
constexpr char32_t PLUS = U'+';

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

std::optional<PropertyParser::Result> PropertyParser::Parse ( char const* html, Stream stream ) noexcept
{
    if ( !stream.ExpectNotEmpty ( html, "pbr::PropertyParser::Parse" ) )
        return std::nullopt;

    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    auto s = ASCIIString::Parse ( html, stream, ASCIIString::eParseMode::LettersAndDashes );

    if ( !s )
        return std::nullopt;

    std::string_view name = s->_target;
    ASCIIString::ToLower ( name );
    auto const findResult = g_properties.find ( name );

    if ( findResult == g_properties.cend () )
    {
        android_vulkan::LogError ( "pbr::PropertyParser::Parse - %s:%zu: Unknown property '%s'.",
            html,
            stream._line,
            std::string ( name ).c_str ()
        );

        return std::nullopt;
    }

    stream = s->_newStream;

    if ( !stream.ExpectNotEmpty ( html, "pbr::PropertyParser::Parse" ) )
        return std::nullopt;

    auto skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;

    if ( !stream.ExpectNotEmpty ( html, "pbr::PropertyParser::Parse" ) )
        return std::nullopt;

    probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character != U':' )
    {
        android_vulkan::LogError ( "pbr::PropertyParser::Parse - %s:%zu: Value expected.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    stream = probe->_newStream;

    if ( !stream.ExpectNotEmpty ( html, "pbr::PropertyParser::Parse" ) )
        return std::nullopt;

    skip = Whitespace::Skip ( html, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;
    std::u32string value {};

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( html, "pbr::AttributeParser::Parse" ) )
            return std::nullopt;

        probe = UTF8Parser::Parse ( html, stream );

        if ( !probe )
            return std::nullopt;

        char32_t const c = probe->_character;
        stream = probe->_newStream;

        if ( c == U';' )
            break;

        value.push_back ( c );
    }

    auto const end = value.crend ();

    auto const check = std::find_if ( value.crbegin (),
        end,

        [] ( char32_t c ) noexcept -> bool {
            return !Whitespace::IsWhitespace ( c );
        }
    );

    auto const size = static_cast<size_t> ( std::distance ( check, end ) );

    if ( size == 0UZ )
    {
        android_vulkan::LogError ( "pbr::PropertyParser::Parse - %s:%zu: Value is empty.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    value.resize ( size );

    return Result
    {
        ._newStream = stream,
        ._property = findResult->second,
        ._value = std::move ( value )
    };
}

char const* PropertyParser::ToString ( Property::eType type ) noexcept
{
    switch ( type )
    {
        case Property::eType::BackgroundColor:
        return "background-color";

        case Property::eType::BackgroundSize:
        return "background-size";

        case Property::eType::Bottom:
        return "bottom";

        case Property::eType::Color:
        return "color";

        case Property::eType::Display:
        return "display";

        case Property::eType::FontFamily:
        return "font-family";

        case Property::eType::FontSize:
        return "font-size";

        case Property::eType::Height:
        return "height";

        case Property::eType::Left:
        return "left";

        case Property::eType::LineHeight:
        return "line-height";

        case Property::eType::Margin:
        return "margin";

        case Property::eType::MarginBottom:
        return "margin-bottom";

        case Property::eType::MarginLeft:
        return "margin-left";

        case Property::eType::MarginRight:
        return "margin-right";

        case Property::eType::MarginTop:
        return "margin-top";

        case Property::eType::Padding:
        return "padding";

        case Property::eType::PaddingBottom:
        return "padding-bottom";

        case Property::eType::PaddingLeft:
        return "padding-left";

        case Property::eType::PaddingRight:
        return "padding-right";

        case Property::eType::PaddingTop:
        return "padding-top";

        case Property::eType::Position:
        return "position";

        case Property::eType::Right:
        return "right";

        case Property::eType::SRC:
        return "src";

        case Property::eType::TextAlign:
        return "text-align";

        case Property::eType::Top:
        return "top";

        case Property::eType::VerticalAlign:
        return "vertical-align";

        case Property::eType::Width:
        return "width";

        default:
            // IMPOSSIBLE
        break;
    }


    return "UNKNOWN";
}

std::list<std::u32string> PropertyParser::TokenizeGeneric ( std::u32string const &value ) noexcept
{
    std::list<std::u32string> result {};

    size_t start = std::numeric_limits<size_t>::max ();
    bool hasStart = false;
    size_t const size = value.size ();

    for ( size_t i = 0UZ; i < size; ++i )
    {
        bool const isWhitespace = Whitespace::IsWhitespace ( value[ i ] );

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

        result.emplace_back ( value.substr ( start, i - start ) );
        hasStart = false;
    }

    if ( hasStart )
        result.emplace_back ( value.substr ( start ) );

    return result;
}

std::list<std::u32string_view> PropertyParser::TokenizeGeneric ( std::u32string_view value ) noexcept
{
    std::list<std::u32string_view> result {};

    size_t start = std::numeric_limits<size_t>::max ();
    bool hasStart = false;
    size_t const size = value.size ();

    for ( size_t i = 0UZ; i < size; ++i )
    {
        bool const isWhitespace = Whitespace::IsWhitespace ( value[ i ] );

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

        result.emplace_back ( value.substr ( start, i - start ) );
        hasStart = false;
    }

    if ( hasStart )
        result.emplace_back ( value.substr ( start ) );

    return result;
}

std::list<std::u32string> PropertyParser::TokenizeMathExpression ( std::u32string const &value ) noexcept
{
    std::list<std::u32string> result {};

    size_t start = std::numeric_limits<size_t>::max ();
    bool hasStart = false;
    size_t const size = value.size ();

    for ( size_t i = 0UZ; i < size; ++i )
    {
        char32_t const c = value[ i ];

        if ( ( c == LEFT_PARENTHESIS ) | ( c == RIGHT_PARENTHESIS ) | ( c == MINUS ) | ( c == PLUS ) )
        {
            if ( hasStart )
                result.push_back ( value.substr ( start, i - start ) );

            result.emplace_back ( 1UZ, c );
            hasStart = false;
            continue;
        }

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

        result.emplace_back ( value.substr ( start, i - start ) );
        hasStart = false;
    }

    if ( hasStart )
        result.emplace_back ( value.substr ( start ) );

    return result;
}

std::list<std::u32string_view> PropertyParser::TokenizeMathExpression ( std::u32string_view value ) noexcept
{
    std::list<std::u32string_view> result {};

    size_t start = std::numeric_limits<size_t>::max ();
    bool hasStart = false;
    size_t const size = value.size ();

    for ( size_t i = 0UZ; i < size; ++i )
    {
        char32_t const c = value[ i ];

        if ( ( c == LEFT_PARENTHESIS ) | ( c == RIGHT_PARENTHESIS ) | ( c == MINUS ) | ( c == PLUS ) )
        {
            if ( hasStart )
                result.push_back ( value.substr ( start, i - start ) );

            result.push_back ( value.substr ( i, 1UZ ) );
            hasStart = false;
            continue;
        }

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

        result.emplace_back ( value.substr ( start, i - start ) );
        hasStart = false;
    }

    if ( hasStart )
        result.emplace_back ( value.substr ( start ) );

    return result;
}

} // namespace pbr
