#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/vertical_align_property_checker.hpp>


namespace pbr {

namespace {

std::unordered_map<std::u32string_view, VerticalAlignProperty::eValue> const VALUES =
{
    { U"bottom", VerticalAlignProperty::eValue::Bottom },
    { U"middle", VerticalAlignProperty::eValue::Middle },
    { U"top", VerticalAlignProperty::eValue::Top },
    { U"inherit", VerticalAlignProperty::eValue::Inherit }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

VerticalAlignPropertyChecker::VerticalAlignPropertyChecker ( char const* css,
    VerticalAlignProperty::eValue &target
) noexcept:
    PropertyChecker ( css, Property::eType::VerticalAlign ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result VerticalAlignPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
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

    android_vulkan::LogError ( "pbr::VerticalAlignPropertyChecker::Process - %s:%zu: Unsupported 'vertical-align' "
        "value '%s'. It should be: %s.",
        _css,
        result._newStream._line,
        UTF8Parser::ToUTF8 ( v )->c_str (),
        supported.c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
