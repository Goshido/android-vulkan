#include <precompiled_headers.hpp>
#include <pbr/length_property_checker.hpp>
#include <pbr/length_value_parser.hpp>


namespace pbr {

LengthPropertyChecker::LengthPropertyChecker ( char const* css,
    Property::eType property,
    LengthValue &target
) noexcept:
    PropertyChecker ( css, property ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result LengthPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    auto const baseCheck = PropertyChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    if ( auto const length = LengthValueParser::Parse ( _css, result._newStream._line, result._value ); length )
    {
        _target = *length;
        return true;
    }

    return std::nullopt;
}

} // namespace pbr
