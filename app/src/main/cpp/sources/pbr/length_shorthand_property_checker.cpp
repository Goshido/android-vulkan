#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/length_shorthand_property_checker.hpp>
#include <pbr/length_value_parser.hpp>


namespace pbr {

LengthShorthandPropertyChecker::LengthShorthandPropertyChecker ( char const* css,
    Property::eType property,
    LengthValue &top,
    LengthValue &right,
    LengthValue &bottom,
    LengthValue &left
) noexcept:
    PropertyChecker ( css, property ),
    _bottom ( bottom ),
    _top ( top ),
    _left ( left ),
    _right ( right )
{
    // NOTHING
}

PropertyChecker::Result LengthShorthandPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    auto const baseCheck = PropertyChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    auto const values = PropertyParser::TokenizeGeneric ( result._value );
    size_t const line = result._newStream._line;

    switch ( values.size () )
    {
        case 1U:
        return Process1Value ( values.front (), line );

        case 2U:
        return Process2Values ( values, line );

        case 3U:
        return Process3Values ( values, line );

        case 4U:
        return Process4Values ( values, line );

        default:
            android_vulkan::LogError ( "pbr::LengthShorthandPropertyChecker::Process - %s:%zu: "
                "'padding' property should contain no more than 4 values.",
                _css,
                result._newStream._line
            );

        return std::nullopt;
    }
}

PropertyChecker::Result LengthShorthandPropertyChecker::Process1Value ( std::u32string_view value,
    size_t line
) noexcept
{
    auto length = LengthValueParser::Parse ( _css, line, value, false );

    if ( !length )
        return std::nullopt;

    _top = std::move ( *length );
    _bottom = _top;
    _left = _top;
    _right = _top;

    return true;
}

PropertyChecker::Result LengthShorthandPropertyChecker::Process2Values ( std::list<std::u32string> const &value,
    size_t line
) noexcept
{
    auto it = value.cbegin ();
    auto length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _top = std::move ( *length );
    _bottom = _top;

    ++it;
    length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _right = std::move ( *length );
    _left = _right;

    return true;
}

PropertyChecker::Result LengthShorthandPropertyChecker::Process3Values ( std::list<std::u32string> const &value,
    size_t line
) noexcept
{
    auto it = value.cbegin ();
    auto length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _top = std::move ( *length );

    ++it;
    length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _right = std::move ( *length );
    _left = _right;

    ++it;
    length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _bottom = std::move ( *length );
    return true;
}

PropertyChecker::Result LengthShorthandPropertyChecker::Process4Values ( std::list<std::u32string> const &value,
    size_t line
) noexcept
{
    auto it = value.cbegin ();
    auto length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _top = std::move ( *length );

    ++it;
    length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _right = std::move ( *length );

    ++it;
    length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _bottom = std::move ( *length );

    ++it;
    length = LengthValueParser::Parse ( _css, line, *it, false );

    if ( !length )
        return std::nullopt;

    _left = std::move ( *length );
    return true;
}

} // namespace pbr
