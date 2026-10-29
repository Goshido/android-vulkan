#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/length_value_parser.hpp>
#include <pbr/number_parser.hpp>
#include <pbr/property_parser.hpp>
#include <pbr/utf8_parser.hpp>


namespace pbr {

namespace {

std::unordered_map<std::u32string_view, LengthValue::eType> const g_types =
{
    { U"em", LengthValue::eType::EM },
    { U"mm", LengthValue::eType::MM },
    { U"pt", LengthValue::eType::PT },
    { U"px", LengthValue::eType::PX },
    { U"%", LengthValue::eType::Percent }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

std::optional<LengthValue> LengthValueParser::Parse ( char const* css,
    size_t line,
    std::u32string_view value,
    bool allowCalc
) noexcept
{
    if ( value == U"auto" )
        return std::optional<LengthValue> { LengthValue ( LengthValue::eType::Auto, 42.0F ) };

    if ( value == U"inherit" )
        return std::optional<LengthValue> { LengthValue ( LengthValue::eType::Inherit, 42.0F ) };

    if ( constexpr std::u32string_view calc = U"calc"; allowCalc & value.starts_with ( calc ) )
        return ParseCalc ( css, line, value.substr ( calc.size () ) );

    auto const number = NumberParser::Parse ( css, line, value );

    if ( !number ) [[unlikely]]
        return std::nullopt;

    if ( auto const findResult = g_types.find ( number->_tail ); findResult != g_types.cend () ) [[likely]]
        return LengthValue ( findResult->second, number->_value );

    ReportError ( css, line, true, number->_tail, "Parse" );
    return std::nullopt;
}

std::optional<LengthValue> LengthValueParser::ParseCalc ( char const* css,
    size_t line,
    std::u32string_view value
) noexcept
{
    std::list<std::u32string_view> const tokens = PropertyParser::TokenizeMathExpression ( value );
    size_t const count = tokens.size ();

    if ( count < 3UZ ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::LengthValueParser::ParseCalc - %s:%zu: Invalid 'calc' expression.",
            css,
            line
        );

        return std::nullopt;
    }

    auto token = tokens.cbegin ();

    if ( *token++ != U"(" ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::LengthValueParser::ParseCalc - %s:%zu: Expected left parenthesis.",
            css,
            line
        );

        return std::nullopt;
    }

    auto const end = --tokens.cend ();

    if ( *end != U")" ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::LengthValueParser::ParseCalc - %s:%zu: Expected right parenthesis.",
            css,
            line
        );

        return std::nullopt;
    }

    constexpr std::u32string_view minus = U"-";
    bool isMinus = *token == minus;

    if ( ( count & 0x0000'0000'0000'0001UZ ) == static_cast<size_t> ( isMinus ) ) [[unlikely]]
    {
        // If first expression token is minus then total tokens must be even. For example: ( -a + b ). Total is 6.
        // If first expression token is not minus then total tokens must be odd. For example: ( a - b ). Total is 5.
        android_vulkan::LogError ( "pbr::LengthValueParser::ParseCalc - %s:%zu: Invalid 'calc' expression.",
            css,
            line
        );

        return std::nullopt;
    }

    if ( isMinus )
        ++token;

    auto firstNumber = NumberParser::Parse ( css, line, *token++ );

    if ( !firstNumber )
        return std::nullopt;

    auto const firstType = g_types.find ( firstNumber->_tail );

    if ( firstType == g_types.cend () ) [[unlikely]]
    {
        ReportError ( css, line, false, firstNumber->_tail, "ParseCalc" );
        return std::nullopt;
    }

    constexpr auto flipSign = [] ( bool flip, float v ) noexcept -> float {
        return std::bit_cast<float> ( ( static_cast<uint32_t> ( flip ) << 31U ) ^ std::bit_cast<uint32_t> ( v ) );
    };

    firstNumber->_value = flipSign ( isMinus, firstNumber->_value );

    std::vector<LengthValue::CalcRest> rest {};
    rest.reserve ( 2UZ );

    constexpr std::u32string_view plus = U"+";

    for ( ; token != end; ++token )
    {
        isMinus = *token == minus;

        if ( !isMinus && *token != plus ) [[unlikely]]
        {
            android_vulkan::LogError ( "pbr::LengthValueParser::ParseCalc - %s:%zu: Unsupported expression. "
                "Only + and - operators are supported.",
                css,
                line
            );

            return std::nullopt;
        }

        auto const number = NumberParser::Parse ( css, line, *( ++token ) );

        if ( !number ) [[unlikely]]
            return std::nullopt;

        auto const findResult = g_types.find ( number->_tail );

        if ( findResult == g_types.cend () ) [[unlikely]]
        {
            ReportError ( css, line, false, number->_tail, "ParseCalc" );
            return std::nullopt;
        }

        rest.push_back (
            {
                ._value = flipSign ( isMinus, number->_value ),
                ._type = findResult->second
            }
        );
    }

    return std::optional<LengthValue> { LengthValue ( firstType->second, firstNumber->_value, std::move ( rest ) ) };
}

void LengthValueParser::ReportError ( char const* css,
    size_t line,
    bool includeSpecialTypes,
    std::u32string_view errorType,
    char const* where
) noexcept
{
    std::string supported {};
    std::string const separator = " | ";

    auto type = g_types.cbegin ();
    supported += *UTF8Parser::ToUTF8 ( type->first );

    for ( auto const end = g_types.cend (); type != end; ++type )
        supported += separator + *UTF8Parser::ToUTF8 ( type->first );

    if ( includeSpecialTypes )
        supported += " | auto | inherit";

    android_vulkan::LogError ( "pbr::LengthValueParser::%s - %s:%zu: Unsupported length unit '%s' was detected. "
        "Supported units: %s.",
        where,
        css,
        line,
        UTF8Parser::ToUTF8 ( errorType )->c_str (),
        supported.c_str ()
    );
}

} // namespace pbr
