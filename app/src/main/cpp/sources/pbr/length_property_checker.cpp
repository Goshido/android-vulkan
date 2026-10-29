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

    auto length = LengthValueParser::Parse ( _css,
        result._newStream._line,
        result._value,
        _property == Property::eType::Width
    );

    if ( !length )
        return std::nullopt;

    _target = std::move ( *length );
    return true;
}

} // namespace pbr
