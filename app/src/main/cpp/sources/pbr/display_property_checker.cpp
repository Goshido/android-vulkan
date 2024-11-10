#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/display_property_checker.hpp>
#include <pbr/utf8_parser.hpp>


namespace pbr {

namespace {

std::unordered_map<std::u32string_view, DisplayProperty::eValue> const VALUES =
{
    { U"block", DisplayProperty::eValue::Block },
    { U"none", DisplayProperty::eValue::None },
    { U"inline-block", DisplayProperty::eValue::InlineBlock }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

DisplayPropertyChecker::DisplayPropertyChecker ( char const* css, DisplayProperty::eValue &target ) noexcept:
    PropertyChecker ( css, Property::eType::Display ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result DisplayPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    auto const baseCheck = PropertyChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    std::u32string_view const v = result._value;

    if ( auto const findResult = VALUES.find ( v ); findResult != VALUES.cend () )
    {
        _target = findResult->second;
        return true;
    }

    std::string supported {};
    constexpr std::string_view separator = " | ";
    constexpr char const* s = separator.data ();

    for ( auto const &value : VALUES )
        supported += *UTF8Parser::ToUTF8 ( value.first ) + s;

    supported.resize ( supported.size () - separator.size () );

    android_vulkan::LogError ( "pbr::DisplayPropertyChecker::Process - %s:%zu: Unsupported 'display' value '%s'. "
        "It should be: %s.",
        _css,
        result._newStream._line,
        UTF8Parser::ToUTF8 ( v )->c_str (),
        supported.c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
