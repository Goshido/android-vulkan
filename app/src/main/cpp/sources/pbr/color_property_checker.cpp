#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/color_property_checker.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

// See https://developer.mozilla.org/en-US/docs/Web/CSS/named-color
std::unordered_map<std::u32string, GXColorRGB> const ColorPropertyChecker::_colorMap =
{
    { U"aliceblue", GXColorRGB ( 0xF0U, 0xF8U, 0xFFU, 1.0F ) },
    { U"antiquewhite", GXColorRGB ( 0xFAU, 0xEBU, 0xD7U, 1.0F ) },
    { U"aqua", GXColorRGB ( 0x00U, 0xFFU, 0xFFU, 1.0F ) },
    { U"aquamarine", GXColorRGB ( 0x7FU, 0xFFU, 0xD4U, 1.0F ) },
    { U"azure", GXColorRGB ( 0xF0U, 0xFFU, 0xFFU, 1.0F ) },
    { U"beige", GXColorRGB ( 0xF5U, 0xF5U, 0xDCU, 1.0F ) },
    { U"bisque", GXColorRGB ( 0xFFU, 0xE4U, 0xC4U, 1.0F ) },
    { U"black", GXColorRGB ( 0x00U, 0x00U, 0x00U, 1.0F ) },
    { U"blanchedalmond", GXColorRGB ( 0xFFU, 0xEBU, 0xCDU, 1.0F ) },
    { U"blue", GXColorRGB ( 0x00U, 0x00U, 0xFFU, 1.0F ) },
    { U"blueviolet", GXColorRGB ( 0x8AU, 0x2BU, 0xE2U, 1.0F ) },
    { U"brown", GXColorRGB ( 0xA5U, 0x2AU, 0x2AU, 1.0F ) },
    { U"burlywood", GXColorRGB ( 0xDEU, 0xB8U, 0x87U, 1.0F ) },
    { U"cadetblue", GXColorRGB ( 0x5FU, 0x9EU, 0xA0U, 1.0F ) },
    { U"chartreuse", GXColorRGB ( 0x7FU, 0xFFU, 0x00U, 1.0F ) },
    { U"chocolate", GXColorRGB ( 0xD2U, 0x69U, 0x1EU, 1.0F ) },
    { U"coral", GXColorRGB ( 0xFFU, 0x7FU, 0x50U, 1.0F ) },
    { U"cornflowerblue", GXColorRGB ( 0x64U, 0x95U, 0xEDU, 1.0F ) },
    { U"cornsilk", GXColorRGB ( 0xFFU, 0xF8U, 0xDCU, 1.0F ) },
    { U"crimson", GXColorRGB ( 0xDCU, 0x14U, 0x3CU, 1.0F ) },
    { U"cyan", GXColorRGB ( 0x00U, 0xFFU, 0xFFU, 1.0F ) },
    { U"darkblue", GXColorRGB ( 0x00U, 0x00U, 0x8BU, 1.0F ) },
    { U"darkcyan", GXColorRGB ( 0x00U, 0x8BU, 0x8BU, 1.0F ) },
    { U"darkgoldenrod", GXColorRGB ( 0xB8U, 0x86U, 0x0BU, 1.0F ) },
    { U"darkgray", GXColorRGB ( 0xA9U, 0xA9U, 0xA9U, 1.0F ) },
    { U"darkgreen", GXColorRGB ( 0x00U, 0x64U, 0x00U, 1.0F ) },
    { U"darkgrey", GXColorRGB ( 0xA9U, 0xA9U, 0xA9U, 1.0F ) },
    { U"darkkhaki", GXColorRGB ( 0xBDU, 0xB7U, 0x6BU, 1.0F ) },
    { U"darkmagenta", GXColorRGB ( 0x8BU, 0x00U, 0x8BU, 1.0F ) },
    { U"darkolivegreen", GXColorRGB ( 0x55U, 0x6BU, 0x2FU, 1.0F ) },
    { U"darkorange", GXColorRGB ( 0xFFU, 0x8CU, 0x00U, 1.0F ) },
    { U"darkorchid", GXColorRGB ( 0x99U, 0x32U, 0xCCU, 1.0F ) },
    { U"darkred", GXColorRGB ( 0x8BU, 0x00U, 0x00U, 1.0F ) },
    { U"darksalmon", GXColorRGB ( 0xE9U, 0x96U, 0x7AU, 1.0F ) },
    { U"darkseagreen", GXColorRGB ( 0x8FU, 0xBCU, 0x8FU, 1.0F ) },
    { U"darkslateblue", GXColorRGB ( 0x48U, 0x3DU, 0x8BU, 1.0F ) },
    { U"darkslategray", GXColorRGB ( 0x2FU, 0x4FU, 0x4FU, 1.0F ) },
    { U"darkslategrey", GXColorRGB ( 0x2FU, 0x4FU, 0x4FU, 1.0F ) },
    { U"darkturquoise", GXColorRGB ( 0x00U, 0xCEU, 0xD1U, 1.0F ) },
    { U"darkviolet", GXColorRGB ( 0x94U, 0x00U, 0xD3U, 1.0F ) },
    { U"deeppink", GXColorRGB ( 0xFFU, 0x14U, 0x93U, 1.0F ) },
    { U"deepskyblue", GXColorRGB ( 0x00U, 0xBFU, 0xFFU, 1.0F ) },
    { U"dimgray", GXColorRGB ( 0x69U, 0x69U, 0x69U, 1.0F ) },
    { U"dimgrey", GXColorRGB ( 0x69U, 0x69U, 0x69U, 1.0F ) },
    { U"dodgerblue", GXColorRGB ( 0x1EU, 0x90U, 0xFFU, 1.0F ) },
    { U"firebrick", GXColorRGB ( 0xB2U, 0x22U, 0x22U, 1.0F ) },
    { U"floralwhite", GXColorRGB ( 0xFFU, 0xFAU, 0xF0U, 1.0F ) },
    { U"forestgreen", GXColorRGB ( 0x22U, 0x8BU, 0x22U, 1.0F ) },
    { U"fuchsia", GXColorRGB ( 0xFFU, 0x00U, 0xFFU, 1.0F ) },
    { U"gainsboro", GXColorRGB ( 0xDCU, 0xDCU, 0xDCU, 1.0F ) },
    { U"ghostwhite", GXColorRGB ( 0xF8U, 0xF8U, 0xFFU, 1.0F ) },
    { U"gold", GXColorRGB ( 0xFFU, 0xD7U, 0x00U, 1.0F ) },
    { U"goldenrod", GXColorRGB ( 0xDAU, 0xA5U, 0x20U, 1.0F ) },
    { U"gray", GXColorRGB ( 0x80U, 0x80U, 0x80U, 1.0F ) },
    { U"green", GXColorRGB ( 0x00U, 0x80U, 0x00U, 1.0F ) },
    { U"greenyellow", GXColorRGB ( 0xADU, 0xFFU, 0x2FU, 1.0F ) },
    { U"grey", GXColorRGB ( 0x80U, 0x80U, 0x80U, 1.0F ) },
    { U"honeydew", GXColorRGB ( 0xF0U, 0xFFU, 0xF0U, 1.0F ) },
    { U"hotpink", GXColorRGB ( 0xFFU, 0x69U, 0xB4U, 1.0F ) },
    { U"indianred", GXColorRGB ( 0xCDU, 0x5CU, 0x5CU, 1.0F ) },
    { U"indigo", GXColorRGB ( 0x4BU, 0x00U, 0x82U, 1.0F ) },
    { U"ivory", GXColorRGB ( 0xFFU, 0xFFU, 0xF0U, 1.0F ) },
    { U"khaki", GXColorRGB ( 0xF0U, 0xE6U, 0x8CU, 1.0F ) },
    { U"lavender", GXColorRGB ( 0xE6U, 0xE6U, 0xFAU, 1.0F ) },
    { U"lavenderblush", GXColorRGB ( 0xFFU, 0xF0U, 0xF5U, 1.0F ) },
    { U"lawngreen", GXColorRGB ( 0x7CU, 0xFCU, 0x00U, 1.0F ) },
    { U"lemonchiffon", GXColorRGB ( 0xFFU, 0xFAU, 0xCDU, 1.0F ) },
    { U"lightblue", GXColorRGB ( 0xADU, 0xD8U, 0xE6U, 1.0F ) },
    { U"lightcoral", GXColorRGB ( 0xF0U, 0x80U, 0x80U, 1.0F ) },
    { U"lightcyan", GXColorRGB ( 0xE0U, 0xFFU, 0xFFU, 1.0F ) },
    { U"lightgoldenrodyellow", GXColorRGB ( 0xFAU, 0xFAU, 0xD2U, 1.0F ) },
    { U"lightgray", GXColorRGB ( 0xD3U, 0xD3U, 0xD3U, 1.0F ) },
    { U"lightgreen", GXColorRGB ( 0x90U, 0xEEU, 0x90U, 1.0F ) },
    { U"lightgrey", GXColorRGB ( 0xD3U, 0xD3U, 0xD3U, 1.0F ) },
    { U"lightpink", GXColorRGB ( 0xFFU, 0xB6U, 0xC1U, 1.0F ) },
    { U"lightsalmon", GXColorRGB ( 0xFFU, 0xA0U, 0x7AU, 1.0F ) },
    { U"lightseagreen", GXColorRGB ( 0x20U, 0xB2U, 0xAAU, 1.0F ) },
    { U"lightskyblue", GXColorRGB ( 0x87U, 0xCEU, 0xFAU, 1.0F ) },
    { U"lightslategray", GXColorRGB ( 0x77U, 0x88U, 0x99U, 1.0F ) },
    { U"lightslategrey", GXColorRGB ( 0x77U, 0x88U, 0x99U, 1.0F ) },
    { U"lightsteelblue", GXColorRGB ( 0xB0U, 0xC4U, 0xDEU, 1.0F ) },
    { U"lightyellow", GXColorRGB ( 0xFFU, 0xFFU, 0xE0U, 1.0F ) },
    { U"lime", GXColorRGB ( 0x00U, 0xFFU, 0x00U, 1.0F ) },
    { U"limegreen", GXColorRGB ( 0x32U, 0xCDU, 0x32U, 1.0F ) },
    { U"linen", GXColorRGB ( 0xFAU, 0xF0U, 0xE6U, 1.0F ) },
    { U"magenta", GXColorRGB ( 0xFFU, 0x00U, 0xFFU, 1.0F ) },
    { U"maroon", GXColorRGB ( 0x80U, 0x00U, 0x00U, 1.0F ) },
    { U"mediumaquamarine", GXColorRGB ( 0x66U, 0xCDU, 0xAAU, 1.0F ) },
    { U"mediumblue", GXColorRGB ( 0x00U, 0x00U, 0xCDU, 1.0F ) },
    { U"mediumorchid", GXColorRGB ( 0xBAU, 0x55U, 0xD3U, 1.0F ) },
    { U"mediumpurple", GXColorRGB ( 0x93U, 0x70U, 0xDBU, 1.0F ) },
    { U"mediumseagreen", GXColorRGB ( 0x3CU, 0xB3U, 0x71U, 1.0F ) },
    { U"mediumslateblue", GXColorRGB ( 0x7BU, 0x68U, 0xEEU, 1.0F ) },
    { U"mediumspringgreen", GXColorRGB ( 0x00U, 0xFAU, 0x9AU, 1.0F ) },
    { U"mediumturquoise", GXColorRGB ( 0x48U, 0xD1U, 0xCCU, 1.0F ) },
    { U"mediumvioletred", GXColorRGB ( 0xC7U, 0x15U, 0x85U, 1.0F ) },
    { U"midnightblue", GXColorRGB ( 0x19U, 0x19U, 0x70U, 1.0F ) },
    { U"mintcream", GXColorRGB ( 0xF5U, 0xFFU, 0xFAU, 1.0F ) },
    { U"mistyrose", GXColorRGB ( 0xFFU, 0xE4U, 0xE1U, 1.0F ) },
    { U"moccasin", GXColorRGB ( 0xFFU, 0xE4U, 0xB5U, 1.0F ) },
    { U"navajowhite", GXColorRGB ( 0xFFU, 0xDEU, 0xADU, 1.0F ) },
    { U"navy", GXColorRGB ( 0x00U, 0x00U, 0x80U, 1.0F ) },
    { U"oldlace", GXColorRGB ( 0xFDU, 0xF5U, 0xE6U, 1.0F ) },
    { U"olive", GXColorRGB ( 0x80U, 0x80U, 0x00U, 1.0F ) },
    { U"olivedrab", GXColorRGB ( 0x6BU, 0x8EU, 0x23U, 1.0F ) },
    { U"orange", GXColorRGB ( 0xFFU, 0xA5U, 0x00U, 1.0F ) },
    { U"orangered", GXColorRGB ( 0xFFU, 0x45U, 0x00U, 1.0F ) },
    { U"orchid", GXColorRGB ( 0xDAU, 0x70U, 0xD6U, 1.0F ) },
    { U"palegoldenrod", GXColorRGB ( 0xEEU, 0xE8U, 0xAAU, 1.0F ) },
    { U"palegreen", GXColorRGB ( 0x98U, 0xFBU, 0x98U, 1.0F ) },
    { U"paleturquoise", GXColorRGB ( 0xAFU, 0xEEU, 0xEEU, 1.0F ) },
    { U"palevioletred", GXColorRGB ( 0xDBU, 0x70U, 0x93U, 1.0F ) },
    { U"papayawhip", GXColorRGB ( 0xFFU, 0xEFU, 0xD5U, 1.0F ) },
    { U"peachpuff", GXColorRGB ( 0xFFU, 0xDAU, 0xB9U, 1.0F ) },
    { U"peru", GXColorRGB ( 0xCDU, 0x85U, 0x3FU, 1.0F ) },
    { U"pink", GXColorRGB ( 0xFFU, 0xC0U, 0xCBU, 1.0F ) },
    { U"plum", GXColorRGB ( 0xDDU, 0xA0U, 0xDDU, 1.0F ) },
    { U"powderblue", GXColorRGB ( 0xB0U, 0xE0U, 0xE6U, 1.0F ) },
    { U"purple", GXColorRGB ( 0x80U, 0x00U, 0x80U, 1.0F ) },
    { U"rebeccapurple", GXColorRGB ( 0x66U, 0x33U, 0x99U, 1.0F ) },
    { U"red", GXColorRGB ( 0xFFU, 0x00U, 0x00U, 1.0F ) },
    { U"rosybrown", GXColorRGB ( 0xBCU, 0x8FU, 0x8FU, 1.0F ) },
    { U"royalblue", GXColorRGB ( 0x41U, 0x69U, 0xE1U, 1.0F ) },
    { U"saddlebrown", GXColorRGB ( 0x8BU, 0x45U, 0x13U, 1.0F ) },
    { U"salmon", GXColorRGB ( 0xFAU, 0x80U, 0x72U, 1.0F ) },
    { U"sandybrown", GXColorRGB ( 0xF4U, 0xA4U, 0x60U, 1.0F ) },
    { U"seagreen", GXColorRGB ( 0x2EU, 0x8BU, 0x57U, 1.0F ) },
    { U"seashell", GXColorRGB ( 0xFFU, 0xF5U, 0xEEU, 1.0F ) },
    { U"sienna", GXColorRGB ( 0xA0U, 0x52U, 0x2DU, 1.0F ) },
    { U"silver", GXColorRGB ( 0xC0U, 0xC0U, 0xC0U, 1.0F ) },
    { U"skyblue", GXColorRGB ( 0x87U, 0xCEU, 0xEBU, 1.0F ) },
    { U"slateblue", GXColorRGB ( 0x6AU, 0x5AU, 0xCDU, 1.0F ) },
    { U"slategray", GXColorRGB ( 0x70U, 0x80U, 0x90U, 1.0F ) },
    { U"slategrey", GXColorRGB ( 0x70U, 0x80U, 0x90U, 1.0F ) },
    { U"snow", GXColorRGB ( 0xFFU, 0xFAU, 0xFAU, 1.0F ) },
    { U"springgreen", GXColorRGB ( 0x00U, 0xFFU, 0x7FU, 1.0F ) },
    { U"steelblue", GXColorRGB ( 0x46U, 0x82U, 0xB4U, 1.0F ) },
    { U"tan", GXColorRGB ( 0xD2U, 0xB4U, 0x8CU, 1.0F ) },
    { U"teal", GXColorRGB ( 0x00U, 0x80U, 0x80U, 1.0F ) },
    { U"thistle", GXColorRGB ( 0xD8U, 0xBFU, 0xD8U, 1.0F ) },
    { U"tomato", GXColorRGB ( 0xFFU, 0x63U, 0x47U, 1.0F ) },
    { U"transparent", GXColorRGB ( 0x00U, 0x00U, 0x00U, 1.0F ) },
    { U"turquoise", GXColorRGB ( 0x40U, 0xE0U, 0xD0U, 1.0F ) },
    { U"violet", GXColorRGB ( 0xEEU, 0x82U, 0xEEU, 1.0F ) },
    { U"wheat", GXColorRGB ( 0xF5U, 0xDEU, 0xB3U, 1.0F ) },
    { U"white", GXColorRGB ( 0xFFU, 0xFFU, 0xFFU, 1.0F ) },
    { U"whitesmoke", GXColorRGB ( 0xF5U, 0xF5U, 0xF5U, 1.0F ) },
    { U"yellow", GXColorRGB ( 0xFFU, 0xFFU, 0x00U, 1.0F ) },
    { U"yellowgreen", GXColorRGB ( 0x9AU, 0xCDU, 0x32U, 1.0F ) }
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

    constexpr auto compare = [] ( char32_t left, char32_t right ) noexcept -> bool {
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

    constexpr auto compare = [] ( char32_t left, char32_t right ) noexcept -> bool {
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
    constexpr auto conv = [] ( uint8_t v ) noexcept -> float {
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
