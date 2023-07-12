#include <logger.hpp>
#include <pbr/ascii_string.hpp>
#include <pbr/stylesheet_rel_attribute_checker.hpp>
#include <pbr/utf8_parser.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cctype>

GX_RESTORE_WARNING_STATE


namespace pbr {

StylesheetRELAttributeChecker::StylesheetRELAttributeChecker ( char const *html ) noexcept:
    AttributeChecker ( html, eAttribute::REL )
{
    // NOTHING
}

AttributeChecker::Result StylesheetRELAttributeChecker::Process ( AttributeParser::Result &result ) noexcept
{
    auto const baseCheck = AttributeChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    auto const v = UTF8Parser::ToUTF8 ( result._value );

    if ( !v )
        return std::nullopt;

    ASCIIString::ToLower ( *v );
    constexpr std::string_view stylesheet = "stylesheet";

    if ( *v == stylesheet )
        return true;

    android_vulkan::LogError ( "pbr::StylesheetRELAttributeChecker::Process - %s:%zu: Value is not 'stylesheet'. "
        "Got: '%s'",
        _html,
        result._newStream._line,
        v->c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
