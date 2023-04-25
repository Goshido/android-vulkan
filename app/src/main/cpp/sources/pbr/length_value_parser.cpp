#include <logger.h>
#include <pbr/length_value_parser.h>
#include <pbr/number_parser.h>
#include <pbr/utf8_parser.h>


namespace pbr {

std::unordered_map<std::u32string_view, LengthValue::eType> LengthValueParser::_types =
{
    { U"em", LengthValue::eType::EM },
    { U"mm", LengthValue::eType::MM },
    { U"pt", LengthValue::eType::PT },
    { U"px", LengthValue::eType::PX },
    { U"%", LengthValue::eType::Percent }
};

std::optional<LengthValue> LengthValueParser::Parse ( char const* css,
    size_t line,
    std::u32string_view value
) noexcept
{
    if ( value == U"auto" )
        return LengthValue ( LengthValue::eType::Auto, 0.0F );

    auto const number = NumberParser::Parse ( css, line, value );

    if ( !number )
        return std::nullopt;

    if ( auto const findResult = _types.find ( number->_tail ); findResult != _types.cend () )
        return LengthValue ( findResult->second, number->_value );

    std::string supported {};
    constexpr std::string_view separator = " | ";
    constexpr char const* s = separator.data ();

    for ( auto const& item : _types )
        supported += *UTF8Parser::ToUTF8 ( item.first ) + s;

    supported.resize ( supported.size () - separator.size () );

    android_vulkan::LogError ( "pbr::LengthValueParser::Parse - %s:%zu: Unsupported length unit '%s' was detected. "
        "Supported units: %s.",
        css,
        line,
        UTF8Parser::ToUTF8 ( number->_tail )->c_str (),
        supported.c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
