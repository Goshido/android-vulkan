#include <logger.hpp>
#include <pbr/unique_attribute_checker.hpp>
#include <pbr/utf8_parser.hpp>


namespace pbr {

UniqueAttributeChecker::UniqueAttributeChecker ( char const *html,
    eAttribute attribute,
    std::u32string &target,
    std::unordered_set<std::u32string> &registry
) noexcept:
    AttributeChecker ( html, attribute ),
    _registry ( registry ),
    _target ( target )
{
    // NOTHING
}

AttributeChecker::Result UniqueAttributeChecker::Process ( AttributeParser::Result &result ) noexcept
{
    auto const baseCheck = AttributeChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    std::u32string &v = result._value;

    if ( _registry.count ( v ) < 1U )
    {
        _registry.insert ( v );
        _target = std::move ( v );
        return true;
    }

    auto const s = UTF8Parser::ToUTF8 ( v );

    if ( !s )
    {
        android_vulkan::LogError ( "pbr::UniqueAttributeChecker::Process - %s:%zu: Attribute '%s' is not unique.",
            _html,
            result._newStream._line,
            AttributeParser::ToString ( result._attribute )
        );

        return std::nullopt;
    }

    android_vulkan::LogError ( "pbr::UniqueAttributeChecker::Process - %s:%zu: Attribute '%s' is not unique ['%s'].",
        _html,
        result._newStream._line,
        AttributeParser::ToString ( result._attribute ),
        s->c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
