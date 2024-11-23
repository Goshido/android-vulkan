#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/property_checker.hpp>


namespace pbr {

PropertyChecker::PropertyChecker ( char const* css, Property::eType property ) noexcept:
    _css ( css ),
    _property ( property )
{
    // NOTHING
}

PropertyChecker::Result PropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    if ( result._property != _property )
        return false;

    if ( !_detected )
    {
        _detected = true;
        return true;
    }

    android_vulkan::LogError ( "pbr::PropertyChecker::Process - %s:%zu: Property '%s' already present.",
        _css,
        result._newStream._line,
        PropertyParser::ToString ( result._property )
    );

    return std::nullopt;
}

bool PropertyChecker::IsDetected () const noexcept
{
    return _detected;
}

} // namespace pbr
