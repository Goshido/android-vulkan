#include <pbr/set_attribute_checker.hpp>


namespace pbr {

SetAttributeChecker::SetAttributeChecker ( char const *html,
    eAttribute attribute,
    std::unordered_set<std::u32string> &target
) noexcept:
    AttributeChecker ( html, attribute ),
    _target ( target )
{
    // NOTHING
}

AttributeChecker::Result SetAttributeChecker::Process ( AttributeParser::Result &result ) noexcept
{
    auto const baseCheck = AttributeChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    auto t = AttributeParser::Tokenize ( _html,
        result._newStream._line,
        AttributeParser::ToString ( _attribute ),
        result._value
    );

    if ( !t )
        return std::nullopt;

    _target = std::move ( *t );
    return true;
}

} // namespace pbr
