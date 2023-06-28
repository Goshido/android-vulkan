#include <file.h>
#include <logger.h>
#include <pbr/src_property_checker.h>
#include <pbr/utf8_parser.h>
#include <pbr/whitespace.h>


namespace pbr {

SRCPropertyChecker::SRCPropertyChecker ( char const* css, std::string &target, std::string const &assetRoot ) noexcept:
    PropertyChecker ( css, Property::eType::SRC ),
    _assetRoot ( assetRoot ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result SRCPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    auto const baseCheck = PropertyChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    std::u32string_view v = result._value;
    constexpr std::u32string_view prefix = U"url(";
    auto segment = v.substr ( 0U, prefix.size () );

    bool same = std::equal ( segment.cbegin (),
        segment.cend (),
        prefix.cbegin (),
        prefix.cend (),

        [] ( char32_t left, char32_t right ) noexcept -> bool {
            return right == static_cast<char32_t> ( std::tolower ( static_cast<int> ( left ) ) );
        }
    );

    if ( !same )
    {
        android_vulkan::LogError ( "pbr::SRCPropertyChecker::Process - %s:%zu: Expected 'url(' in 'src' value.",
            _css,
            result._newStream._line
        );

        return std::nullopt;
    }

    v = Whitespace::Skip ( v.substr ( prefix.size () ) );

    if ( v.empty () )
    {
        android_vulkan::LogError ( "pbr::SRCPropertyChecker::Process - %s:%zu: Expected resource path in 'src' value.",
            _css,
            result._newStream._line
        );

        return std::nullopt;
    }

    char32_t const c = v.front ();

    auto const parse = ( ( c == U'\'' ) | ( c == U'"' ) ) ?
        ParseQuoted ( result._newStream._line, v.substr ( 1U ), c ) :
        ParseUnquoted ( result._newStream._line, v );

    if ( !parse )
        return std::nullopt;

    v = parse->_tail;

    if ( v.empty () || v.front () != U')' )
    {
        android_vulkan::LogError ( "pbr::SRCPropertyChecker::Process - %s:%zu: ')' in 'src' value.",
            _css,
            result._newStream._line
        );

        return std::nullopt;
    }

    v = Whitespace::Skip ( v.substr ( 1U ) );

    if ( !v.empty () )
    {
        android_vulkan::LogError ( "pbr::SRCPropertyChecker::Process - %s:%zu: Expected nothing after 'url(...)' in "
            "'src' value.",
            _css,
            result._newStream._line
        );

        return std::nullopt;
    }

    auto const r = UTF8Parser::ToUTF8 ( parse->_resource );

    if ( !r )
    {
        android_vulkan::LogError ( "pbr::SRCPropertyChecker::Process - %s:%zu: Can't convert resource to UTF-8 string "
            "'src' value.",
            _css,
            result._newStream._line
        );

        return std::nullopt;
    }

    std::string path = _assetRoot + '/' + *r;

    if ( android_vulkan::File ( path ).IsExist () )
    {
        _target = std::move ( path );
        return true;
    }

    android_vulkan::LogError ( "pbr::SRCPropertyChecker::Process - %s:%zu: Can't locate resource '%s' in "
        "'src' value.",
        _css,
        result._newStream._line,
        path.c_str ()
    );

    return std::nullopt;
}

SRCPropertyChecker::ParseResult SRCPropertyChecker::ParseQuoted ( size_t line,
    std::u32string_view value,
    char32_t quote
) noexcept
{
    if ( auto const idx = value.find ( quote ); idx != std::u32string_view::npos )
    {
        return ParseInfo
        {
            ._resource = value.substr ( 0U, idx ),
            ._tail = Whitespace::Skip ( value.substr ( idx + 1U ) )
        };
    }

    android_vulkan::LogError ( "pbr::SRCPropertyChecker::ParseQuoted - %s:%zu: Can't find closing quote mark in "
        "'src' value.",
        _css,
        line
    );

    return std::nullopt;
}

SRCPropertyChecker::ParseResult SRCPropertyChecker::ParseUnquoted ( size_t line, std::u32string_view value ) noexcept
{
    auto const begin = value.cbegin ();
    auto const end = value.cend ();
    bool hasQuote = false;

    auto const findResult = std::find_if ( begin,
        end,

        [ &hasQuote ] ( char32_t c ) mutable noexcept -> bool {
            hasQuote |= ( c == U'\'' ) | ( c == U'"' );
            return Whitespace::IsWhitespace ( c ) | ( c == U')' );
        }
    );

    if ( hasQuote )
    {
        android_vulkan::LogError ( "pbr::SRCPropertyChecker::ParseUnquoted - %s:%zu: It's expected unquoted "
            "resource path in 'src' value.",
            _css,
            line
        );

        return std::nullopt;
    }

    if ( findResult == end )
    {
        android_vulkan::LogError ( "pbr::SRCPropertyChecker::ParseUnquoted - %s:%zu: Expected resource path "
            "in 'src' value.",
            _css,
            line
        );

        return std::nullopt;
    }

    auto const idx = static_cast<size_t> ( std::distance ( begin, findResult ) );

    return ParseInfo
    {
        ._resource = value.substr ( 0U, idx ),
        ._tail = Whitespace::Skip ( value.substr ( idx ) )
    };
}

} // namespace pbr
