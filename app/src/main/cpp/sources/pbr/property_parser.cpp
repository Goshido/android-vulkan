#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/ascii_string.hpp>
#include <pbr/property_parser.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

std::unordered_map<std::string_view, Property::eType> const PropertyParser::_properties =
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

std::unordered_map<Property::eType, std::string_view> const PropertyParser::_names =
{
    { Property::eType::BackgroundColor, "background-color" },
    { Property::eType::BackgroundSize, "background-size" },
    { Property::eType::Bottom, "bottom" },
    { Property::eType::Color, "color" },
    { Property::eType::Display, "display" },
    { Property::eType::FontFamily, "font-family" },
    { Property::eType::FontSize, "font-size" },
    { Property::eType::Height, "height" },
    { Property::eType::Left, "left" },
    { Property::eType::Margin, "margin" },
    { Property::eType::MarginBottom, "margin-bottom" },
    { Property::eType::MarginLeft, "margin-left" },
    { Property::eType::MarginRight, "margin-right" },
    { Property::eType::MarginTop, "margin-top" },
    { Property::eType::Padding, "padding" },
    { Property::eType::PaddingBottom, "padding-bottom" },
    { Property::eType::PaddingLeft, "padding-left" },
    { Property::eType::PaddingRight, "padding-right" },
    { Property::eType::PaddingTop, "padding-top" },
    { Property::eType::Position, "position" },
    { Property::eType::Right, "right" },
    { Property::eType::SRC, "src" },
    { Property::eType::TextAlign, "text-align" },
    { Property::eType::Top, "top" },
    { Property::eType::VerticalAlign, "vertical-align" },
    { Property::eType::Width, "width" }
};

std::optional<PropertyParser::Result> PropertyParser::Parse ( char const* html, Stream stream ) noexcept
{
    if ( !stream.ExpectNotEmpty ( html, "pbr::PropertyParser::Parse" ) )
        return std::nullopt;

    auto probe = UTF8Parser::Parse ( html, stream );

    if ( !probe )
        return std::nullopt;

    char32_t c = probe->_character;
    auto s = ASCIIString::Parse ( html, stream, ASCIIString::eParseMode::LettersAndDashes );

    if ( !s )
        return std::nullopt;

    std::string_view name = s->_target;
    ASCIIString::ToLower ( name );
    auto const findResult = _properties.find ( name );

    if ( findResult == _properties.cend () )
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

        c = probe->_character;
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

    if ( size == 0U )
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
    return _names.find ( type )->second.data ();
}

std::list<std::u32string> PropertyParser::Tokenize ( std::u32string const &value ) noexcept
{
    std::list<std::u32string> result {};

    size_t start = std::numeric_limits<size_t>::max ();
    bool hasStart = false;
    size_t const size = value.size ();

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

        result.emplace_back ( value.substr ( start, i - start ) );
        hasStart = false;
    }

    if ( hasStart )
        result.emplace_back ( value.substr ( start ) );

    return result;
}

} // namespace pbr
