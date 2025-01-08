#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/line_height_property_checker.hpp>
#include <pbr/number_parser.hpp>
#include <pbr/utf8_parser.hpp>


namespace pbr {

namespace {

std::unordered_map<std::u32string_view, LengthValue::eType> g_types =
{
    { U"em", LengthValue::eType::EM },
    { U"mm", LengthValue::eType::MM },
    { U"pt", LengthValue::eType::PT },
    { U"px", LengthValue::eType::PX },
    { U"%", LengthValue::eType::Percent }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

LineHeightPropertyChecker::LineHeightPropertyChecker ( char const* css, LengthValue &target ) noexcept:
    PropertyChecker ( css, Property::eType::LineHeight ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result LineHeightPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    auto const baseCheck = PropertyChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    if ( result._value == U"normal" )
    {
        _target = LengthValue ( LengthValue::eType::Auto, 42.0F );
        return true;
    }

    if ( result._value == U"inherit" )
    {
        _target = LengthValue ( LengthValue::eType::Inherit, 42.0F );
        return true;
    }

    auto const number = NumberParser::Parse ( _css, result._newStream._line, result._value );

    if ( !number )
        return std::nullopt;

    if ( number->_tail.empty () )
    {
        _target = LengthValue ( LengthValue::eType::Unitless, number->_value );
        return true;
    }

    if ( auto const findResult = g_types.find ( number->_tail ); findResult != g_types.cend () )
    {
        _target = LengthValue ( findResult->second, number->_value );
        return true;
    }

    std::string supported {};
    constexpr std::string_view separator = " | ";
    constexpr char const* s = separator.data ();

    for ( auto const &item : g_types )
        supported += *UTF8Parser::ToUTF8 ( item.first ) + s;

    supported += "inherit | normal | <unitless number>";

    android_vulkan::LogError ( "pbr::LineHeightPropertyChecker::Parse - %s:%zu: Unsupported unit '%s' "
        "was detected. Supported units: %s.",
        _css,
        result._newStream._line,
        UTF8Parser::ToUTF8 ( number->_tail )->c_str (),
        supported.c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
