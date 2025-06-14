#include <precompiled_headers.hpp>
#include <pbr/utf16_parser.hpp>


namespace pbr {

namespace {

constexpr uint32_t EXTRACT_CODE_POINT_MASK = 0b0000'0011'1111'1111U;

constexpr uint32_t EXTRACT_SURROGATE_MASK = 0b1111'1100'0000'0000U;
constexpr uint32_t HI_SURROGATE_MASK = 0b1101'1000'0000'0000U;
constexpr uint32_t LOW_SURROGATE_MASK = 0b1101'1100'0000'0000U;

constexpr uint32_t SURROGATE_RANGE_FROM = 0b0000'0000'0000'0001'0000'0000'0000'0000U;
constexpr uint32_t SURROGATE_RANGE_TO = 0b0000'0000'0001'0000'1111'1111'1111'1111U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UTF16Parser::eSurrogate UTF16Parser::Classify ( char16_t c ) noexcept
{
    switch ( static_cast<uint32_t> ( c ) & EXTRACT_SURROGATE_MASK )
    {
        case HI_SURROGATE_MASK:
        return eSurrogate::Hi;

        case LOW_SURROGATE_MASK:
        return eSurrogate::Low;

        default:
            // NOTHING
        break;
    }

    return eSurrogate::None;
}

char32_t UTF16Parser::ToChar32 ( char16_t highSurrogate, char16_t lowSurrogate ) noexcept
{
    /*
    Based on https://en.wikipedia.org/wiki/UTF-16#U+0000_to_U+D7FF_and_U+E000_to_U+FFFF

    W1 = 0000'0000'0000'0000'1101'10yy'yyyy'yyyy      0xD800 + yy'yyyy'yyyy
    W2 = 0000'0000'0000'0000'1101'11xx'xxxx'xxxx      0xDC00 + xx'xxxx'xxxx
    U' = 0000'0000'0000'yyyy'yyyy'yyxx'xxxx'xxxx      U - 0x0001'0000
         0000'0000'0000'0001'0000'0000'0000'0000      0x0001'0000
    U  = 0000'0000'000?'????'yyyy'yyxx'xxxx'xxxx      U' + 0x0001'0000
    */

    return static_cast<char32_t> (
        SURROGATE_RANGE_FROM +
        ( 
            ( static_cast<uint32_t> ( lowSurrogate ) & EXTRACT_CODE_POINT_MASK ) |
            ( ( highSurrogate & EXTRACT_CODE_POINT_MASK ) << 10U )
        )
    );
}

std::u16string UTF16Parser::ToU16String ( std::u32string_view string ) noexcept
{
    std::u16string result {};

    // Worst case estimation.
    result.reserve ( string.size () * 2U );

    for ( char32_t const c : string )
    {
        /*
        Based on https://en.wikipedia.org/wiki/UTF-16#Code_points_from_U+010000_to_U+10FFFF

        U  = 0000'0000'000?'????'yyyy'yyxx'xxxx'xxxx      U' + 0x0001'0000
             0000'0000'0000'0001'0000'0000'0000'0000      0x0001'0000
        U' = 0000'0000'0000'yyyy'yyyy'yyxx'xxxx'xxxx      U - 0x0001'0000
        W1 = 0000'0000'0000'0000'1101'10yy'yyyy'yyyy      0xD800 + yy'yyyy'yyyy
        W2 = 0000'0000'0000'0000'1101'11xx'xxxx'xxxx      0xDC00 + xx'xxxx'xxxx
        */

        auto const u = static_cast<uint32_t> ( c );

        char16_t const cases[ 2U ][ 2U ] =
        {
            { 
                static_cast<char16_t> ( c ),
                0
            },
            { 
                static_cast<char16_t> ( HI_SURROGATE_MASK | ( ( u - SURROGATE_RANGE_FROM ) >> 10U ) ),
                static_cast<char16_t> ( LOW_SURROGATE_MASK | ( u & EXTRACT_CODE_POINT_MASK ) )
            }
        };

        auto const selector = static_cast<size_t> ( ( u >= SURROGATE_RANGE_FROM ) & ( u <= SURROGATE_RANGE_TO ) );
        result.append ( cases[ selector ], 1U + selector );
    }

    return result;
}

} // namespace pbr
