#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/number_parser.hpp>


namespace pbr {

std::optional<NumberParser::Result> NumberParser::Parse ( char const* css,
    size_t line,
    std::u32string_view value
) noexcept
{
    size_t size = 0UZ;
    constexpr size_t bufferSize = 128UZ;
    char buffer[ bufferSize ];

    for ( ; !value.empty (); value = value.substr ( 1U ) )
    {
        char32_t const c = value.front ();

        if ( !std::isdigit ( static_cast<int> ( c ) ) & ( c != U'-' ) & ( c != U'.' ) )
            break;

        if ( size < bufferSize ) [[likely]]
        {
            buffer[ size++ ] = static_cast<char> ( c );
            continue;
        }

        android_vulkan::LogError ( "pbr::NumberParser::Parse - %s:%zu: Number value is too long.", css, line );
        return std::nullopt;
    }

    if ( size < 1UZ ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::NumberParser::Parse - %s:%zu: Number value is expected.", css, line );
        return std::nullopt;
    }

    char const *end = buffer + size;
    float v;
    auto const [ptr, status] = std::from_chars ( buffer, end, v, std::chars_format::general );
    constexpr std::errc ok {};

    if ( ( status != ok ) | ( ptr != end ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::NumberParser::Parse - %s:%zu: Can not parse number value.", css, line );
        return std::nullopt;
    }

    return std::optional<NumberParser::Result> {
        Result
        {
            ._tail = value,
            ._value = v
        }
    };
}

} // namespace pbr
