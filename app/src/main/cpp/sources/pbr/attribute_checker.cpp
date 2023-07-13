#include <logger.hpp>
#include <pbr/attribute_checker.hpp>


namespace pbr {

AttributeChecker::AttributeChecker ( char const* html, eAttribute attribute ) noexcept:
    _attribute ( attribute ),
    _html ( html )
{
    // NOTHING
}

bool AttributeChecker::IsDetected () const noexcept
{
    return _detected;
}

AttributeChecker::Result AttributeChecker::Process ( AttributeParser::Result &result ) noexcept
{
    if ( result._attribute != _attribute )
        return false;

    if ( !_detected )
    {
        _detected = true;
        return true;
    }

    android_vulkan::LogError ( "pbr::AttributeChecker::Process - %s:%zu: Attribute '%s' already present.",
        _html,
        result._newStream._line,
        AttributeParser::ToString ( result._attribute )
    );

    return std::nullopt;
}

} // namespace pbr
