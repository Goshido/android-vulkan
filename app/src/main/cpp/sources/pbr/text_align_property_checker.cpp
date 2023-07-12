#include <logger.hpp>
#include <pbr/text_align_property_checker.hpp>
#include <pbr/utf8_parser.hpp>


namespace pbr {

namespace {

std::unordered_map<std::u32string_view, TextAlignProperty::eValue> const VALUES =
{
    { U"center", TextAlignProperty::eValue::Center },
    { U"left", TextAlignProperty::eValue::Left },
    { U"right", TextAlignProperty::eValue::Right },
    { U"inherit", TextAlignProperty::eValue::Inherit }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

TextAlignPropertyChecker::TextAlignPropertyChecker ( char const* css, TextAlignProperty::eValue &target ) noexcept:
    PropertyChecker ( css, Property::eType::TextAlign ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result TextAlignPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
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

    android_vulkan::LogError ( "pbr::TextAlignPropertyChecker::Process - %s:%zu: Unsupported 'text-align' value '%s'. "
        "It should be: %s.",
        _css,
        result._newStream._line,
        UTF8Parser::ToUTF8 ( v )->c_str (),
        supported.c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
