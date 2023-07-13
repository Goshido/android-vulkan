#include <logger.hpp>
#include <pbr/fail_property_checker.hpp>


namespace pbr {

FailPropertyChecker::FailPropertyChecker ( char const* css, Property::eType property ) noexcept:
    PropertyChecker ( css, property )
{
    // NOTHING
}

PropertyChecker::Result FailPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    android_vulkan::LogError ( "pbr::FailPropertyChecker::Process - %s:%zu: Unexpected property '%s'.",
        _css,
        result._newStream._line,
        PropertyParser::ToString ( _property )
    );

    return std::nullopt;
}

} // namespace pbr
