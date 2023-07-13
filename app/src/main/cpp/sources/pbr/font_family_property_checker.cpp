#include <logger.hpp>
#include <pbr/font_family_property_checker.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

FontFamilyPropertyChecker::FontFamilyPropertyChecker ( char const* css, std::u32string &target ) noexcept:
    PropertyChecker ( css, Property::eType::FontFamily ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result FontFamilyPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    auto const baseCheck = PropertyChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    std::u32string_view const v = result._value;
    char32_t const c = v.front ();

    if ( ( c == U'\'' ) | ( c == U'"' ) )
        return ParseQuoted ( result._newStream._line, v.substr ( 1U ), c );

    return ParseUnquoted ( result._newStream._line, v );
}

PropertyChecker::Result FontFamilyPropertyChecker::ParseQuoted ( size_t line,
    std::u32string_view value,
    char32_t quote
) noexcept
{
    std::u32string_view const start = value;

    for ( ; ; )
    {
        if ( value.empty () || ( quote == value.front () ) )
            break;

        value = value.substr ( 1U );
    }

    if ( value.size () == 1U )
    {
        _target = start.substr ( 0U, start.size () - 1U );
        return true;
    }

    android_vulkan::LogError ( "pbr::FontFamilyPropertyChecker::ParseQuoted - %s:%zu: There is no end quotation mark "
        "in 'font-family' value.",
        _css,
        line
    );

    return std::nullopt;
}

PropertyChecker::Result FontFamilyPropertyChecker::ParseUnquoted ( size_t line,
    std::u32string_view value) noexcept
{
    std::u32string_view const start = value;
    value = value.substr ( 1U );

    for ( ; ; )
    {
        if ( value.empty () )
            break;

        char32_t const c = value.front ();

        if ( Whitespace::IsWhitespace ( c ) )
            break;

        if ( ( c != U'\'' ) & ( c != U'"' ) )
        {
            value = value.substr ( 1U );
            continue;
        }

        android_vulkan::LogError ( "pbr::FontFamilyPropertyChecker::ParseUnquoted - %s:%zu: Unexpected quotation mark "
            "in 'font-family' value.",
            _css,
            line
        );

        return std::nullopt;
    }

    value = Whitespace::Skip ( value );

    if ( value.empty () )
    {
        _target = start;
        return true;
    }

    android_vulkan::LogError ( "pbr::FontFamilyPropertyChecker::ParseUnquoted - %s:%zu: Expected unquoted "
        "'font-family' value.",
        _css,
        line
    );

    return std::nullopt;
}

} // namespace pbr
