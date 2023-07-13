#include <logger.hpp>
#include <pbr/number_parser.hpp>


namespace pbr {

std::optional<NumberParser::Result> NumberParser::Parse ( char const* css,
    size_t line,
    std::u32string_view value
) noexcept
{
    bool digit = false;
    bool dot = false;
    bool sign = false;
    size_t size = 0U;

    constexpr size_t bufferSize = 128U;
    char buffer[ bufferSize ];

    auto const append = [ & ] ( char32_t c ) noexcept -> bool
    {
        constexpr size_t safeSize = bufferSize - 1U;
        if ( size < safeSize )
        {
            buffer[ size++ ] = static_cast<char> ( c );
            return true;
        }

        android_vulkan::LogError ( "pbr::NumberParser::Parse - %s:%zu: NUmber value is too long.", css, line );
        return false;
    };

    for ( ; ; )
    {
        if ( value.empty () )
            break;

        char32_t const c = value.front ();

        if ( std::isdigit ( static_cast<int> ( c ) ) )
        {
            if ( !append ( c ) )
                return std::nullopt;

            digit = true;
            value = value.substr ( 1U );
            continue;
        }

        if ( c == U'-' )
        {
            if ( sign )
            {
                android_vulkan::LogError ( "pbr::NumberParser::Parse - %s:%zu: Expected number value. "
                    "Unexpected '-' symbol was detected.",
                    css,
                    line
                );

                return std::nullopt;
            }

            if ( !append ( c ) )
                return std::nullopt;

            sign = true;
            value = value.substr ( 1U );
            continue;
        }

        if ( c != U'.' )
            break;

        if ( dot )
        {
            android_vulkan::LogError ( "pbr::NumberParser::Parse - %s:%zu: Expected number value. "
                "Unexpected '.' symbol was detected.",
                css,
                line
            );

            return std::nullopt;
        }

        if ( !append ( c ) )
            return std::nullopt;

        dot = true;
        value = value.substr ( 1U );
    }

    if ( ( size == 0U ) | !digit )
    {
        android_vulkan::LogError ( "pbr::NumberParser::Parse - %s:%zu: Numerical value is expected.", css, line );
        return std::nullopt;
    }

    buffer[ size ] = 0;

    return Result
    {
        ._tail = value,
        ._value = static_cast<float> ( std::atof ( buffer ) )
    };
}

} // namespace pbr
