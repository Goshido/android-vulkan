#include <logger.hpp>
#include <pbr/color_property_checker.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

namespace {

[[nodiscard]] consteval GXUByte Conv ( uint32_t v ) noexcept
{
    return static_cast<GXUByte> ( v );
}

} // end of anonymous namespace

// See https://developer.mozilla.org/en-US/docs/Web/CSS/named-color
std::unordered_map<std::u32string, GXColorRGB> const ColorPropertyChecker::_colorMap =
{
    { U"aliceblue", GXColorRGB ( Conv ( 0xF0U ), Conv ( 0xF8U ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"antiquewhite", GXColorRGB ( Conv ( 0xFAU ), Conv ( 0xEBU ), Conv ( 0xD7U ), Conv ( 0xFFU ) ) },
    { U"aqua", GXColorRGB ( Conv ( 0x00U ), Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"aquamarine", GXColorRGB ( Conv ( 0x7FU ), Conv ( 0xFFU ), Conv ( 0xD4U ), Conv ( 0xFFU ) ) },
    { U"azure", GXColorRGB ( Conv ( 0xF0U ), Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"beige", GXColorRGB ( Conv ( 0xF5U ), Conv ( 0xF5U ), Conv ( 0xDCU ), Conv ( 0xFFU ) ) },
    { U"bisque", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xE4U ), Conv ( 0xC4U ), Conv ( 0xFFU ) ) },
    { U"black", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"blanchedalmond", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xEBU ), Conv ( 0xCDU ), Conv ( 0xFFU ) ) },
    { U"blue", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"blueviolet", GXColorRGB ( Conv ( 0x8AU ), Conv ( 0x2BU ), Conv ( 0xE2U ), Conv ( 0xFFU ) ) },
    { U"brown", GXColorRGB ( Conv ( 0xA5U ), Conv ( 0x2AU ), Conv ( 0x2AU ), Conv ( 0xFFU ) ) },
    { U"burlywood", GXColorRGB ( Conv ( 0xDEU ), Conv ( 0xB8U ), Conv ( 0x87U ), Conv ( 0xFFU ) ) },
    { U"cadetblue", GXColorRGB ( Conv ( 0x5FU ), Conv ( 0x9EU ), Conv ( 0xA0U ), Conv ( 0xFFU ) ) },
    { U"chartreuse", GXColorRGB ( Conv ( 0x7FU ), Conv ( 0xFFU ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"chocolate", GXColorRGB ( Conv ( 0xD2U ), Conv ( 0x69U ), Conv ( 0x1EU ), Conv ( 0xFFU ) ) },
    { U"coral", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x7FU ), Conv ( 0x50U ), Conv ( 0xFFU ) ) },
    { U"cornflowerblue", GXColorRGB ( Conv ( 0x64U ), Conv ( 0x95U ), Conv ( 0xEDU ), Conv ( 0xFFU ) ) },
    { U"cornsilk", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xF8U ), Conv ( 0xDCU ), Conv ( 0xFFU ) ) },
    { U"crimson", GXColorRGB ( Conv ( 0xDCU ), Conv ( 0x14U ), Conv ( 0x3CU ), Conv ( 0xFFU ) ) },
    { U"cyan", GXColorRGB ( Conv ( 0x00U ), Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"darkblue", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0x8BU ), Conv ( 0xFFU ) ) },
    { U"darkcyan", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x8BU ), Conv ( 0x8BU ), Conv ( 0xFFU ) ) },
    { U"darkgoldenrod", GXColorRGB ( Conv ( 0xB8U ), Conv ( 0x86U ), Conv ( 0x0BU ), Conv ( 0xFFU ) ) },
    { U"darkgray", GXColorRGB ( Conv ( 0xA9U ), Conv ( 0xA9U ), Conv ( 0xA9U ), Conv ( 0xFFU ) ) },
    { U"darkgreen", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x64U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"darkgrey", GXColorRGB ( Conv ( 0xA9U ), Conv ( 0xA9U ), Conv ( 0xA9U ), Conv ( 0xFFU ) ) },
    { U"darkkhaki", GXColorRGB ( Conv ( 0xBDU ), Conv ( 0xB7U ), Conv ( 0x6BU ), Conv ( 0xFFU ) ) },
    { U"darkmagenta", GXColorRGB ( Conv ( 0x8BU ), Conv ( 0x00U ), Conv ( 0x8BU ), Conv ( 0xFFU ) ) },
    { U"darkolivegreen", GXColorRGB ( Conv ( 0x55U ), Conv ( 0x6BU ), Conv ( 0x2FU ), Conv ( 0xFFU ) ) },
    { U"darkorange", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x8CU ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"darkorchid", GXColorRGB ( Conv ( 0x99U ), Conv ( 0x32U ), Conv ( 0xCCU ), Conv ( 0xFFU ) ) },
    { U"darkred", GXColorRGB ( Conv ( 0x8BU ), Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"darksalmon", GXColorRGB ( Conv ( 0xE9U ), Conv ( 0x96U ), Conv ( 0x7AU ), Conv ( 0xFFU ) ) },
    { U"darkseagreen", GXColorRGB ( Conv ( 0x8FU ), Conv ( 0xBCU ), Conv ( 0x8FU ), Conv ( 0xFFU ) ) },
    { U"darkslateblue", GXColorRGB ( Conv ( 0x48U ), Conv ( 0x3DU ), Conv ( 0x8BU ), Conv ( 0xFFU ) ) },
    { U"darkslategray", GXColorRGB ( Conv ( 0x2FU ), Conv ( 0x4FU ), Conv ( 0x4FU ), Conv ( 0xFFU ) ) },
    { U"darkslategrey", GXColorRGB ( Conv ( 0x2FU ), Conv ( 0x4FU ), Conv ( 0x4FU ), Conv ( 0xFFU ) ) },
    { U"darkturquoise", GXColorRGB ( Conv ( 0x00U ), Conv ( 0xCEU ), Conv ( 0xD1U ), Conv ( 0xFFU ) ) },
    { U"darkviolet", GXColorRGB ( Conv ( 0x94U ), Conv ( 0x00U ), Conv ( 0xD3U ), Conv ( 0xFFU ) ) },
    { U"deeppink", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x14U ), Conv ( 0x93U ), Conv ( 0xFFU ) ) },
    { U"deepskyblue", GXColorRGB ( Conv ( 0x00U ), Conv ( 0xBFU ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"dimgray", GXColorRGB ( Conv ( 0x69U ), Conv ( 0x69U ), Conv ( 0x69U ), Conv ( 0xFFU ) ) },
    { U"dimgrey", GXColorRGB ( Conv ( 0x69U ), Conv ( 0x69U ), Conv ( 0x69U ), Conv ( 0xFFU ) ) },
    { U"dodgerblue", GXColorRGB ( Conv ( 0x1EU ), Conv ( 0x90U ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"firebrick", GXColorRGB ( Conv ( 0xB2U ), Conv ( 0x22U ), Conv ( 0x22U ), Conv ( 0xFFU ) ) },
    { U"floralwhite", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xFAU ), Conv ( 0xF0U ), Conv ( 0xFFU ) ) },
    { U"forestgreen", GXColorRGB ( Conv ( 0x22U ), Conv ( 0x8BU ), Conv ( 0x22U ), Conv ( 0xFFU ) ) },
    { U"fuchsia", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x00U ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"gainsboro", GXColorRGB ( Conv ( 0xDCU ), Conv ( 0xDCU ), Conv ( 0xDCU ), Conv ( 0xFFU ) ) },
    { U"ghostwhite", GXColorRGB ( Conv ( 0xF8U ), Conv ( 0xF8U ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"gold", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xD7U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"goldenrod", GXColorRGB ( Conv ( 0xDAU ), Conv ( 0xA5U ), Conv ( 0x20U ), Conv ( 0xFFU ) ) },
    { U"gray", GXColorRGB ( Conv ( 0x80U ), Conv ( 0x80U ), Conv ( 0x80U ), Conv ( 0xFFU ) ) },
    { U"green", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x80U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"greenyellow", GXColorRGB ( Conv ( 0xADU ), Conv ( 0xFFU ), Conv ( 0x2FU ), Conv ( 0xFFU ) ) },
    { U"grey", GXColorRGB ( Conv ( 0x80U ), Conv ( 0x80U ), Conv ( 0x80U ), Conv ( 0xFFU ) ) },
    { U"honeydew", GXColorRGB ( Conv ( 0xF0U ), Conv ( 0xFFU ), Conv ( 0xF0U ), Conv ( 0xFFU ) ) },
    { U"hotpink", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x69U ), Conv ( 0xB4U ), Conv ( 0xFFU ) ) },
    { U"indianred", GXColorRGB ( Conv ( 0xCDU ), Conv ( 0x5CU ), Conv ( 0x5CU ), Conv ( 0xFFU ) ) },
    { U"indigo", GXColorRGB ( Conv ( 0x4BU ), Conv ( 0x00U ), Conv ( 0x82U ), Conv ( 0xFFU ) ) },
    { U"ivory", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xF0U ), Conv ( 0xFFU ) ) },
    { U"khaki", GXColorRGB ( Conv ( 0xF0U ), Conv ( 0xE6U ), Conv ( 0x8CU ), Conv ( 0xFFU ) ) },
    { U"lavender", GXColorRGB ( Conv ( 0xE6U ), Conv ( 0xE6U ), Conv ( 0xFAU ), Conv ( 0xFFU ) ) },
    { U"lavenderblush", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xF0U ), Conv ( 0xF5U ), Conv ( 0xFFU ) ) },
    { U"lawngreen", GXColorRGB ( Conv ( 0x7CU ), Conv ( 0xFCU ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"lemonchiffon", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xFAU ), Conv ( 0xCDU ), Conv ( 0xFFU ) ) },
    { U"lightblue", GXColorRGB ( Conv ( 0xADU ), Conv ( 0xD8U ), Conv ( 0xE6U ), Conv ( 0xFFU ) ) },
    { U"lightcoral", GXColorRGB ( Conv ( 0xF0U ), Conv ( 0x80U ), Conv ( 0x80U ), Conv ( 0xFFU ) ) },
    { U"lightcyan", GXColorRGB ( Conv ( 0xE0U ), Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"lightgoldenrodyellow", GXColorRGB ( Conv ( 0xFAU ), Conv ( 0xFAU ), Conv ( 0xD2U ), Conv ( 0xFFU ) ) },
    { U"lightgray", GXColorRGB ( Conv ( 0xD3U ), Conv ( 0xD3U ), Conv ( 0xD3U ), Conv ( 0xFFU ) ) },
    { U"lightgreen", GXColorRGB ( Conv ( 0x90U ), Conv ( 0xEEU ), Conv ( 0x90U ), Conv ( 0xFFU ) ) },
    { U"lightgrey", GXColorRGB ( Conv ( 0xD3U ), Conv ( 0xD3U ), Conv ( 0xD3U ), Conv ( 0xFFU ) ) },
    { U"lightpink", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xB6U ), Conv ( 0xC1U ), Conv ( 0xFFU ) ) },
    { U"lightsalmon", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xA0U ), Conv ( 0x7AU ), Conv ( 0xFFU ) ) },
    { U"lightseagreen", GXColorRGB ( Conv ( 0x20U ), Conv ( 0xB2U ), Conv ( 0xAAU ), Conv ( 0xFFU ) ) },
    { U"lightskyblue", GXColorRGB ( Conv ( 0x87U ), Conv ( 0xCEU ), Conv ( 0xFAU ), Conv ( 0xFFU ) ) },
    { U"lightslategray", GXColorRGB ( Conv ( 0x77U ), Conv ( 0x88U ), Conv ( 0x99U ), Conv ( 0xFFU ) ) },
    { U"lightslategrey", GXColorRGB ( Conv ( 0x77U ), Conv ( 0x88U ), Conv ( 0x99U ), Conv ( 0xFFU ) ) },
    { U"lightsteelblue", GXColorRGB ( Conv ( 0xB0U ), Conv ( 0xC4U ), Conv ( 0xDEU ), Conv ( 0xFFU ) ) },
    { U"lightyellow", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xE0U ), Conv ( 0xFFU ) ) },
    { U"lime", GXColorRGB ( Conv ( 0x00U ), Conv ( 0xFFU ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"limegreen", GXColorRGB ( Conv ( 0x32U ), Conv ( 0xCDU ), Conv ( 0x32U ), Conv ( 0xFFU ) ) },
    { U"linen", GXColorRGB ( Conv ( 0xFAU ), Conv ( 0xF0U ), Conv ( 0xE6U ), Conv ( 0xFFU ) ) },
    { U"magenta", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x00U ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"maroon", GXColorRGB ( Conv ( 0x80U ), Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"mediumaquamarine", GXColorRGB ( Conv ( 0x66U ), Conv ( 0xCDU ), Conv ( 0xAAU ), Conv ( 0xFFU ) ) },
    { U"mediumblue", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0xCDU ), Conv ( 0xFFU ) ) },
    { U"mediumorchid", GXColorRGB ( Conv ( 0xBAU ), Conv ( 0x55U ), Conv ( 0xD3U ), Conv ( 0xFFU ) ) },
    { U"mediumpurple", GXColorRGB ( Conv ( 0x93U ), Conv ( 0x70U ), Conv ( 0xDBU ), Conv ( 0xFFU ) ) },
    { U"mediumseagreen", GXColorRGB ( Conv ( 0x3CU ), Conv ( 0xB3U ), Conv ( 0x71U ), Conv ( 0xFFU ) ) },
    { U"mediumslateblue", GXColorRGB ( Conv ( 0x7BU ), Conv ( 0x68U ), Conv ( 0xEEU ), Conv ( 0xFFU ) ) },
    { U"mediumspringgreen", GXColorRGB ( Conv ( 0x00U ), Conv ( 0xFAU ), Conv ( 0x9AU ), Conv ( 0xFFU ) ) },
    { U"mediumturquoise", GXColorRGB ( Conv ( 0x48U ), Conv ( 0xD1U ), Conv ( 0xCCU ), Conv ( 0xFFU ) ) },
    { U"mediumvioletred", GXColorRGB ( Conv ( 0xC7U ), Conv ( 0x15U ), Conv ( 0x85U ), Conv ( 0xFFU ) ) },
    { U"midnightblue", GXColorRGB ( Conv ( 0x19U ), Conv ( 0x19U ), Conv ( 0x70U ), Conv ( 0xFFU ) ) },
    { U"mintcream", GXColorRGB ( Conv ( 0xF5U ), Conv ( 0xFFU ), Conv ( 0xFAU ), Conv ( 0xFFU ) ) },
    { U"mistyrose", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xE4U ), Conv ( 0xE1U ), Conv ( 0xFFU ) ) },
    { U"moccasin", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xE4U ), Conv ( 0xB5U ), Conv ( 0xFFU ) ) },
    { U"navajowhite", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xDEU ), Conv ( 0xADU ), Conv ( 0xFFU ) ) },
    { U"navy", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0x80U ), Conv ( 0xFFU ) ) },
    { U"oldlace", GXColorRGB ( Conv ( 0xFDU ), Conv ( 0xF5U ), Conv ( 0xE6U ), Conv ( 0xFFU ) ) },
    { U"olive", GXColorRGB ( Conv ( 0x80U ), Conv ( 0x80U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"olivedrab", GXColorRGB ( Conv ( 0x6BU ), Conv ( 0x8EU ), Conv ( 0x23U ), Conv ( 0xFFU ) ) },
    { U"orange", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xA5U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"orangered", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x45U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"orchid", GXColorRGB ( Conv ( 0xDAU ), Conv ( 0x70U ), Conv ( 0xD6U ), Conv ( 0xFFU ) ) },
    { U"palegoldenrod", GXColorRGB ( Conv ( 0xEEU ), Conv ( 0xE8U ), Conv ( 0xAAU ), Conv ( 0xFFU ) ) },
    { U"palegreen", GXColorRGB ( Conv ( 0x98U ), Conv ( 0xFBU ), Conv ( 0x98U ), Conv ( 0xFFU ) ) },
    { U"paleturquoise", GXColorRGB ( Conv ( 0xAFU ), Conv ( 0xEEU ), Conv ( 0xEEU ), Conv ( 0xFFU ) ) },
    { U"palevioletred", GXColorRGB ( Conv ( 0xDBU ), Conv ( 0x70U ), Conv ( 0x93U ), Conv ( 0xFFU ) ) },
    { U"papayawhip", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xEFU ), Conv ( 0xD5U ), Conv ( 0xFFU ) ) },
    { U"peachpuff", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xDAU ), Conv ( 0xB9U ), Conv ( 0xFFU ) ) },
    { U"peru", GXColorRGB ( Conv ( 0xCDU ), Conv ( 0x85U ), Conv ( 0x3FU ), Conv ( 0xFFU ) ) },
    { U"pink", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xC0U ), Conv ( 0xCBU ), Conv ( 0xFFU ) ) },
    { U"plum", GXColorRGB ( Conv ( 0xDDU ), Conv ( 0xA0U ), Conv ( 0xDDU ), Conv ( 0xFFU ) ) },
    { U"powderblue", GXColorRGB ( Conv ( 0xB0U ), Conv ( 0xE0U ), Conv ( 0xE6U ), Conv ( 0xFFU ) ) },
    { U"purple", GXColorRGB ( Conv ( 0x80U ), Conv ( 0x00U ), Conv ( 0x80U ), Conv ( 0xFFU ) ) },
    { U"rebeccapurple", GXColorRGB ( Conv ( 0x66U ), Conv ( 0x33U ), Conv ( 0x99U ), Conv ( 0xFFU ) ) },
    { U"red", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"rosybrown", GXColorRGB ( Conv ( 0xBCU ), Conv ( 0x8FU ), Conv ( 0x8FU ), Conv ( 0xFFU ) ) },
    { U"royalblue", GXColorRGB ( Conv ( 0x41U ), Conv ( 0x69U ), Conv ( 0xE1U ), Conv ( 0xFFU ) ) },
    { U"saddlebrown", GXColorRGB ( Conv ( 0x8BU ), Conv ( 0x45U ), Conv ( 0x13U ), Conv ( 0xFFU ) ) },
    { U"salmon", GXColorRGB ( Conv ( 0xFAU ), Conv ( 0x80U ), Conv ( 0x72U ), Conv ( 0xFFU ) ) },
    { U"sandybrown", GXColorRGB ( Conv ( 0xF4U ), Conv ( 0xA4U ), Conv ( 0x60U ), Conv ( 0xFFU ) ) },
    { U"seagreen", GXColorRGB ( Conv ( 0x2EU ), Conv ( 0x8BU ), Conv ( 0x57U ), Conv ( 0xFFU ) ) },
    { U"seashell", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xF5U ), Conv ( 0xEEU ), Conv ( 0xFFU ) ) },
    { U"sienna", GXColorRGB ( Conv ( 0xA0U ), Conv ( 0x52U ), Conv ( 0x2DU ), Conv ( 0xFFU ) ) },
    { U"silver", GXColorRGB ( Conv ( 0xC0U ), Conv ( 0xC0U ), Conv ( 0xC0U ), Conv ( 0xFFU ) ) },
    { U"skyblue", GXColorRGB ( Conv ( 0x87U ), Conv ( 0xCEU ), Conv ( 0xEBU ), Conv ( 0xFFU ) ) },
    { U"slateblue", GXColorRGB ( Conv ( 0x6AU ), Conv ( 0x5AU ), Conv ( 0xCDU ), Conv ( 0xFFU ) ) },
    { U"slategray", GXColorRGB ( Conv ( 0x70U ), Conv ( 0x80U ), Conv ( 0x90U ), Conv ( 0xFFU ) ) },
    { U"slategrey", GXColorRGB ( Conv ( 0x70U ), Conv ( 0x80U ), Conv ( 0x90U ), Conv ( 0xFFU ) ) },
    { U"snow", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xFAU ), Conv ( 0xFAU ), Conv ( 0xFFU ) ) },
    { U"springgreen", GXColorRGB ( Conv ( 0x00U ), Conv ( 0xFFU ), Conv ( 0x7FU ), Conv ( 0xFFU ) ) },
    { U"steelblue", GXColorRGB ( Conv ( 0x46U ), Conv ( 0x82U ), Conv ( 0xB4U ), Conv ( 0xFFU ) ) },
    { U"tan", GXColorRGB ( Conv ( 0xD2U ), Conv ( 0xB4U ), Conv ( 0x8CU ), Conv ( 0xFFU ) ) },
    { U"teal", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x80U ), Conv ( 0x80U ), Conv ( 0xFFU ) ) },
    { U"thistle", GXColorRGB ( Conv ( 0xD8U ), Conv ( 0xBFU ), Conv ( 0xD8U ), Conv ( 0xFFU ) ) },
    { U"tomato", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0x63U ), Conv ( 0x47U ), Conv ( 0xFFU ) ) },
    { U"transparent", GXColorRGB ( Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0x00U ), Conv ( 0x00U ) ) },
    { U"turquoise", GXColorRGB ( Conv ( 0x40U ), Conv ( 0xE0U ), Conv ( 0xD0U ), Conv ( 0xFFU ) ) },
    { U"violet", GXColorRGB ( Conv ( 0xEEU ), Conv ( 0x82U ), Conv ( 0xEEU ), Conv ( 0xFFU ) ) },
    { U"wheat", GXColorRGB ( Conv ( 0xF5U ), Conv ( 0xDEU ), Conv ( 0xB3U ), Conv ( 0xFFU ) ) },
    { U"white", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0xFFU ) ) },
    { U"whitesmoke", GXColorRGB ( Conv ( 0xF5U ), Conv ( 0xF5U ), Conv ( 0xF5U ), Conv ( 0xFFU ) ) },
    { U"yellow", GXColorRGB ( Conv ( 0xFFU ), Conv ( 0xFFU ), Conv ( 0x00U ), Conv ( 0xFFU ) ) },
    { U"yellowgreen", GXColorRGB ( Conv ( 0x9AU ), Conv ( 0xCDU ), Conv ( 0x32U ), Conv ( 0xFFU ) ) }
};

ColorPropertyChecker::ColorPropertyChecker ( char const* css, Property::eType property, ColorValue &target ) noexcept:
    PropertyChecker ( css, property ),
    _target ( target )
{
    // NOTHING
}

PropertyChecker::Result ColorPropertyChecker::Process ( PropertyParser::Result &result ) noexcept
{
    auto const baseCheck = PropertyChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    std::u32string_view value = result._value;

    if ( value == U"inherit" )
    {
        _target = ColorValue ( true, GXColorRGB ( 0.0F, 0.0F, 0.0F, 0.0F ) );
        return true;
    }

    if ( auto const findResult = _colorMap.find ( result._value ); findResult != _colorMap.cend () )
    {
        _target = ColorValue ( false, findResult->second );
        return true;
    }

    char32_t const c = value.front ();
    size_t const l = result._newStream._line;

    if ( c == U'#' )
        return HandleHEXColor ( value.substr ( 1U ), l );

    if ( auto const res = HandleRGBColor ( value, l ); res == std::nullopt || *res )
        return res;

    if ( auto const res = HandleHSLColor ( value, l ); res == std::nullopt || *res )
        return res;

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::Process - %s:%zu: Only named color, HEX, rgb, rgba, hsl "
        "and hsla color formats are supported.",
        _css,
        l
    );

    return std::nullopt;
}

PropertyChecker::Result ColorPropertyChecker::HandleHEXColor ( std::u32string_view value, size_t line ) noexcept
{
    if ( value.size () == 6U )
        return HandleHEXColor6 ( value, line );

    if ( value.size () == 8U )
        return HandleHEXColor8 ( value, line );

    if ( value.size () == 3U )
        return HandleHEXColor3 ( value, line );

    if ( value.size () == 4U )
        return HandleHEXColor4 ( value, line );

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleHEXColor - %s:%zu: Expected HEX color with "
        "3, 4, 6 or 8 symbols.",
        _css,
        line
    );

    return std::nullopt;
}

PropertyChecker::Result ColorPropertyChecker::HandleHEXColor3 ( std::u32string_view value, size_t line ) noexcept
{
    auto const r = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 0U ] ) ) );
    auto const g = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 1U ] ) ) );
    auto const b = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 2U ] ) ) );

    bool const c0 = IsHEX ( r );
    bool const c1 = IsHEX ( g );
    bool const c2 = IsHEX ( b );

    if ( c0 & c1 & c2 )
    {
        _target = ColorValue ( false, GXColorRGB ( ToComponent ( r ), ToComponent ( g ), ToComponent ( b ), 1.0F ) );
        return true;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleHEXColor3 - %s:%zu: Expected HEX color with "
        "3 symbols.",
        _css,
        line
    );

    return std::nullopt;
}

PropertyChecker::Result ColorPropertyChecker::HandleHEXColor4 ( std::u32string_view value, size_t line ) noexcept
{
    auto const r = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 0U ] ) ) );
    auto const g = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 1U ] ) ) );
    auto const b = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 2U ] ) ) );
    auto const a = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 3U ] ) ) );

    bool const c0 = IsHEX ( r );
    bool const c1 = IsHEX ( g );
    bool const c2 = IsHEX ( b );
    bool const c3 = IsHEX ( a );

    if ( c0 & c1 & c2 & c3 )
    {
        _target = ColorValue ( false,
            GXColorRGB ( ToComponent ( r ), ToComponent ( g ), ToComponent ( b ), ToComponent ( a ) )
        );

        return true;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleHEXColor3 - %s:%zu: Expected HEX color with "
        "4 symbols.",
        _css,
        line
    );

    return std::nullopt;
}

PropertyChecker::Result ColorPropertyChecker::HandleHEXColor6 ( std::u32string_view value, size_t line ) noexcept
{
    auto const rH = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 0U ] ) ) );
    auto const rL = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 1U ] ) ) );
    auto const gH = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 2U ] ) ) );
    auto const gL = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 3U ] ) ) );
    auto const bH = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 4U ] ) ) );
    auto const bL = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 5U ] ) ) );

    bool const c0H = IsHEX ( rH );
    bool const c0L = IsHEX ( rL );
    bool const c1H = IsHEX ( gH );
    bool const c1L = IsHEX ( gL );
    bool const c2H = IsHEX ( bH );
    bool const c2L = IsHEX ( bL );

    if ( c0H & c0L & c1H & c1L & c2H & c2L )
    {
        _target = ColorValue ( false,
            GXColorRGB ( ToComponent ( rH, rL ), ToComponent ( gH, gL ), ToComponent ( bH, bL ), 1.0F )
        );

        return true;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleHEXColor3 - %s:%zu: Expected HEX color with "
        "6 symbols.",
        _css,
        line
    );

    return std::nullopt;
}

PropertyChecker::Result ColorPropertyChecker::HandleHEXColor8 ( std::u32string_view value, size_t line ) noexcept
{
    auto const rH = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 0U ] ) ) );
    auto const rL = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 1U ] ) ) );
    auto const gH = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 2U ] ) ) );
    auto const gL = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 3U ] ) ) );
    auto const bH = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 4U ] ) ) );
    auto const bL = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 5U ] ) ) );
    auto const aH = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 6U ] ) ) );
    auto const aL = static_cast<char32_t> ( std::tolower ( static_cast<int> ( value[ 7U ] ) ) );

    bool const c0H = IsHEX ( rH );
    bool const c0L = IsHEX ( rL );
    bool const c1H = IsHEX ( gH );
    bool const c1L = IsHEX ( gL );
    bool const c2H = IsHEX ( bH );
    bool const c2L = IsHEX ( bL );
    bool const c3H = IsHEX ( aH );
    bool const c3L = IsHEX ( aL );

    if ( c0H & c0L & c1H & c1L & c2H & c2L & c3H & c3L )
    {
        _target = ColorValue ( false,
            GXColorRGB ( ToComponent ( rH, rL ),
                ToComponent ( gH, gL ),
                ToComponent ( bH, bL ),
                ToComponent ( aH, aL )
            )
        );

        return true;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleHEXColor3 - %s:%zu: Expected HEX color with "
        "8 symbols.",
        _css,
        line
    );

    return std::nullopt;
}

PropertyChecker::Result ColorPropertyChecker::HandleHSLColor ( std::u32string_view value, size_t line ) noexcept
{
    constexpr std::u32string_view hslPrefix = U"hsl(";
    constexpr size_t hslPrefixSize = hslPrefix .size ();
    auto segment = value.substr ( 0U, hslPrefixSize );
    size_t prefixSize = hslPrefixSize;

    auto const compare = [] ( char32_t left, char32_t right ) noexcept -> bool {
        return right == static_cast<char32_t> ( std::tolower ( static_cast<int> ( left ) ) );
    };

    bool same = std::equal ( segment.cbegin (),
        segment.cend (),
        hslPrefix.cbegin (),
        hslPrefix.cend (),
        compare
    );

    if ( !same )
    {
        constexpr std::u32string_view hslaPrefix = U"hsla(";
        constexpr size_t hslaPrefixSize = hslaPrefix.size ();
        segment = value.substr ( 0U, hslaPrefixSize );

        same = std::equal ( segment.cbegin (),
            segment.cend (),
            hslaPrefix.cbegin (),
            hslaPrefix.cend (),
            compare
        );

        prefixSize = hslaPrefixSize;
    }

    if ( !same )
        return false;

    value = Whitespace::Skip ( value.substr ( prefixSize ) );

    if ( value.empty () )
    {
        android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleHSLColor - %s:%zu: Expected color value.",
            _css,
            line
        );

        return std::nullopt;
    }

    GXVec4 color {};
    auto p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    auto number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 0U, 0.0F, 360.0F, 1.0F, 3.6F ) )
        return std::nullopt;

    value = p->_tail;

    if ( value.front () == U',' )
        value = value.substr ( 1U );

    p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 1U, 0.0F, 100.0F, 1.0e-2F, 1.0e-2F ) )
        return std::nullopt;

    value = p->_tail;

    if ( value.front () == U',' )
        value = value.substr ( 1U );

    p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 2U, 0.0F, 100.0F, 1.0e-2F, 1.0e-2F ) )
        return std::nullopt;

    value = p->_tail;

    // See https://en.wikipedia.org/wiki/HSL_and_HSV
    float const h = color._data[ 0U ];
    float const l = color._data[ 2U ];
    float const a = color._data[ 1U ] * std::min ( l, 1.0F - l );

    auto const conv = [ h, l, a ] ( float n ) noexcept -> float {
        constexpr float b = 1.0F / 30.0F;
        float const k = std::fmod ( n + h * b, 12.0F );
        return l - a * std::max ( -1.0F, std::min ( 1.0F, std::min ( k - 3.0F, 9.0F - k ) ) );
    };

    color._data[ 0U ] = conv ( 0.0F );
    color._data[ 1U ] = conv ( 8.0F );
    color._data[ 2U ] = conv ( 4.0F );

    if ( value == U")" )
    {
        _target = ColorValue ( false, GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ) );
        return true;
    }

    // Alpha component parsing.

    if ( value.front () == U',' )
        value = value.substr ( 1U );

    p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 3U, 0.0F, 1.0F, 1.0F, 1.0e-2F ) )
        return std::nullopt;

    value = p->_tail;

    if ( value == U")" )
    {
        _target = ColorValue ( false,
            GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], color._data[ 3U ] )
        );

        return true;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleHSLColor - %s:%zu: Expected 4 component color value.",
        _css,
        line
    );

    return std::nullopt;
}

PropertyChecker::Result ColorPropertyChecker::HandleRGBColor ( std::u32string_view value, size_t line ) noexcept
{
    constexpr std::u32string_view rgbPrefix = U"rgb(";
    constexpr size_t rgbPrefixSize = rgbPrefix.size ();
    auto segment = value.substr ( 0U, rgbPrefixSize );
    size_t prefixSize = rgbPrefixSize;

    auto const compare = [] ( char32_t left, char32_t right ) noexcept -> bool {
        return right == static_cast<char32_t> ( std::tolower ( static_cast<int> ( left ) ) );
    };

    bool same = std::equal ( segment.cbegin (),
        segment.cend (),
        rgbPrefix.cbegin (),
        rgbPrefix.cend (),
        compare
    );

    if ( !same )
    {
        constexpr std::u32string_view rgbaPrefix = U"rgba(";
        constexpr size_t rgbaPrefixSize = rgbaPrefix.size ();
        segment = value.substr ( 0U, rgbaPrefixSize );

        same = std::equal ( segment.cbegin (),
            segment.cend (),
            rgbaPrefix.cbegin (),
            rgbaPrefix.cend (),
            compare
        );

        prefixSize = rgbaPrefixSize;
    }

    if ( !same )
        return false;

    value = Whitespace::Skip ( value.substr ( prefixSize ) );

    if ( value.empty () )
    {
        android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleRGBColor - %s:%zu: Expected color value.",
            _css,
            line
        );

        return std::nullopt;
    }

    GXVec4 color {};
    auto p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    auto number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 0U, 0.0F, 255.0F, GX_MATH_UNORM_FACTOR, 1.0e-2F ) )
        return std::nullopt;

    value = p->_tail;

    if ( value.front () == U',' )
        value = value.substr ( 1U );

    p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 1U, 0.0F, 255.0F, GX_MATH_UNORM_FACTOR, 1.0e-2F ) )
        return std::nullopt;

    value = p->_tail;

    if ( value.front () == U',' )
        value = value.substr ( 1U );

    p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 2U, 0.0F, 255.0F, GX_MATH_UNORM_FACTOR, 1.0e-2F ) )
        return std::nullopt;

    value = p->_tail;

    if ( value == U")" )
    {
        _target = ColorValue ( false, GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ) );
        return true;
    }

    // Alpha component parsing.

    if ( value.front () == U',' )
        value = value.substr ( 1U );

    p = ParseParameter ( line, value );

    if ( !p )
        return std::nullopt;

    number = NumberParser::Parse ( _css, line, p->_value );

    if ( !Convert ( color, number, line, 3U, 0.0F, 1.0F, 1.0F, 1.0e-2F ) )
        return std::nullopt;

    value = p->_tail;

    if ( value == U")" )
    {
        _target = ColorValue ( false,
            GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], color._data[ 3U ] )
        );

        return true;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleRGBColor - %s:%zu: Expected 4 component color value.",
        _css,
        line
    );

    return std::nullopt;
}

bool ColorPropertyChecker::Convert ( GXVec4 &color,
    std::optional<NumberParser::Result> const &number,
    size_t line,
    size_t component,
    float pureMin,
    float pureMax,
    float pureScale,
    float percentScale
) const noexcept
{
    if ( !number )
        return false;

    std::u32string_view tail = number->_tail;
    float const v = number->_value;

    if ( tail.empty () )
    {
        if ( ( v >= pureMin ) & ( v <= pureMax ) )
        {
            color._data[ component ] = v * pureScale;
            return true;
        }

        android_vulkan::LogError ( "pbr::ColorPropertyChecker::Convert - %s:%zu: Incorrect %zu "
            "parameter of color value. Parameter should be in range [%g, %g].",
            _css,
            line,
            component + 1U,
            pureMin,
            pureMax
        );

        return false;
    }

    if ( tail == U"%" )
    {
        if ( ( v >= 0.0F ) & ( v <= 100.0F ) )
        {
            color._data[ component ] = v * percentScale;
            return true;
        }

        android_vulkan::LogError ( "pbr::ColorPropertyChecker::Convert - %s:%zu: Incorrect %zu "
            "parameter of color value. Parameter should be in range [0.0%%, 100.0%%].",
            _css,
            line,
            component + 1U
        );

        return false;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::Convert - %s:%zu: Incorrect %zu parameter "
        "of color value. Only pure and percent values are supported.",
        _css,
        line,
        component + 1U
    );

    return false;
}

uint8_t ColorPropertyChecker::FromHEX ( char32_t c ) noexcept
{
    uint8_t const cases[] =
    {
        static_cast<uint8_t> ( c - U'0' ),
        static_cast<uint8_t> ( 10 + ( c - U'a' ) )
    };

    return cases[ static_cast<size_t> ( c >= U'a' ) ];
}

bool ColorPropertyChecker::IsHEX ( char32_t c ) noexcept
{
    return ( ( c >= U'0' ) & ( c <= U'9' ) ) | ( ( c >= U'a' ) & ( c <= U'f' ) );
}

ColorPropertyChecker::ParseResult ColorPropertyChecker::ParseParameter ( size_t line,
    std::u32string_view value
) noexcept
{
    value = Whitespace::Skip ( value );

    if ( value.empty () )
    {
        android_vulkan::LogError ( "pbr::ColorPropertyChecker::ParseParameter - %s:%zu: Incorrect color value "
            "(branch 1).",
            _css,
            line
        );

        return std::nullopt;
    }

    char32_t const* v = value.data ();
    value = value.substr ( 1U );

    auto const begin = value.cbegin ();
    auto const end = value.cend ();

    auto const findResult = std::find_if ( begin,
        end,

        [] ( char32_t c ) noexcept -> bool {
            return Whitespace::IsWhitespace ( c ) | ( c == U')' ) | ( c == U',' );
        }
    );

    if ( findResult == end )
    {
        android_vulkan::LogError ( "pbr::ColorPropertyChecker::ParseParameter - %s:%zu: Incorrect color value "
            "(branch 2).",
            _css,
            line
        );

        return std::nullopt;
    }

    auto const idx = static_cast<size_t> ( std::distance ( begin, findResult ) );
    value = Whitespace::Skip ( value.substr ( idx ) );

    if ( !value.empty () )
    {
        return ParseInfo
        {
            ._value = std::u32string_view ( v, idx + 1U ),
            ._tail = value
        };
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::ParseParameter - %s:%zu: Incorrect color value (branch 3).",
        _css,
        line
    );

    return std::nullopt;
}

float ColorPropertyChecker::ToComponent ( char32_t c ) noexcept
{
    auto const conv = [] ( uint8_t v ) noexcept -> float {
        constexpr float const lut[] =
        {
            0.0F,
            0.066667F,
            0.13333F,
            0.2F,
            0.26667F,
            0.33333F,
            0.4F,
            0.46667F,
            0.53333F,
            0.6F,
            0.66667F,
            0.73333F,
            0.8F,
            0.86667F,
            0.93333F,
            1.0F
        };

        return lut[ static_cast<size_t> ( v ) ];
    };

    return conv ( FromHEX ( c ) );
}

float ColorPropertyChecker::ToComponent ( char32_t high, char32_t low ) noexcept
{
    return GX_MATH_UNORM_FACTOR * static_cast<float> ( FromHEX ( low ) | ( FromHEX ( high ) << UINT8_C ( 4 ) ) );
}

} // namespace pbr
