#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/color_property_checker.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

namespace {

constexpr uint8_t OPAQUE_VALUE = 0xFFU;

} // end of anonymous namespace

// See https://developer.mozilla.org/en-US/docs/Web/CSS/named-color
std::unordered_map<std::u32string, GXColorUNORM> const ColorPropertyChecker::_colorMap =
{
    { U"aliceblue", { 0xF0U, 0xF8U, 0xFFU, 0xFFU } },
    { U"antiquewhite", { 0xFAU, 0xEBU, 0xD7U, 0xFFU } },
    { U"aqua", { 0x00U, 0xFFU, 0xFFU, 0xFFU } },
    { U"aquamarine", { 0x7FU, 0xFFU, 0xD4U, 0xFFU } },
    { U"azure", { 0xF0U, 0xFFU, 0xFFU, 0xFFU } },
    { U"beige", { 0xF5U, 0xF5U, 0xDCU, 0xFFU } },
    { U"bisque", { 0xFFU, 0xE4U, 0xC4U, 0xFFU } },
    { U"black", { 0x00U, 0x00U, 0x00U, 0xFFU } },
    { U"blanchedalmond", { 0xFFU, 0xEBU, 0xCDU, 0xFFU } },
    { U"blue", { 0x00U, 0x00U, 0xFFU, 0xFFU } },
    { U"blueviolet", { 0x8AU, 0x2BU, 0xE2U, 0xFFU } },
    { U"brown", { 0xA5U, 0x2AU, 0x2AU, 0xFFU } },
    { U"burlywood", { 0xDEU, 0xB8U, 0x87U, 0xFFU } },
    { U"cadetblue", { 0x5FU, 0x9EU, 0xA0U, 0xFFU } },
    { U"chartreuse", { 0x7FU, 0xFFU, 0x00U, 0xFFU } },
    { U"chocolate", { 0xD2U, 0x69U, 0x1EU, 0xFFU } },
    { U"coral", { 0xFFU, 0x7FU, 0x50U, 0xFFU } },
    { U"cornflowerblue", { 0x64U, 0x95U, 0xEDU, 0xFFU } },
    { U"cornsilk", { 0xFFU, 0xF8U, 0xDCU, 0xFFU } },
    { U"crimson", { 0xDCU, 0x14U, 0x3CU, 0xFFU } },
    { U"cyan", { 0x00U, 0xFFU, 0xFFU, 0xFFU } },
    { U"darkblue", { 0x00U, 0x00U, 0x8BU, 0xFFU } },
    { U"darkcyan", { 0x00U, 0x8BU, 0x8BU, 0xFFU } },
    { U"darkgoldenrod", { 0xB8U, 0x86U, 0x0BU, 0xFFU } },
    { U"darkgray", { 0xA9U, 0xA9U, 0xA9U, 0xFFU } },
    { U"darkgreen", { 0x00U, 0x64U, 0x00U, 0xFFU } },
    { U"darkgrey", { 0xA9U, 0xA9U, 0xA9U, 0xFFU } },
    { U"darkkhaki", { 0xBDU, 0xB7U, 0x6BU, 0xFFU } },
    { U"darkmagenta", { 0x8BU, 0x00U, 0x8BU, 0xFFU } },
    { U"darkolivegreen", { 0x55U, 0x6BU, 0x2FU, 0xFFU } },
    { U"darkorange", { 0xFFU, 0x8CU, 0x00U, 0xFFU } },
    { U"darkorchid", { 0x99U, 0x32U, 0xCCU, 0xFFU } },
    { U"darkred", { 0x8BU, 0x00U, 0x00U, 0xFFU } },
    { U"darksalmon", { 0xE9U, 0x96U, 0x7AU, 0xFFU } },
    { U"darkseagreen", { 0x8FU, 0xBCU, 0x8FU, 0xFFU } },
    { U"darkslateblue", { 0x48U, 0x3DU, 0x8BU, 0xFFU } },
    { U"darkslategray", { 0x2FU, 0x4FU, 0x4FU, 0xFFU } },
    { U"darkslategrey", { 0x2FU, 0x4FU, 0x4FU, 0xFFU } },
    { U"darkturquoise", { 0x00U, 0xCEU, 0xD1U, 0xFFU } },
    { U"darkviolet", { 0x94U, 0x00U, 0xD3U, 0xFFU } },
    { U"deeppink", { 0xFFU, 0x14U, 0x93U, 0xFFU } },
    { U"deepskyblue", { 0x00U, 0xBFU, 0xFFU, 0xFFU } },
    { U"dimgray", { 0x69U, 0x69U, 0x69U, 0xFFU } },
    { U"dimgrey", { 0x69U, 0x69U, 0x69U, 0xFFU } },
    { U"dodgerblue", { 0x1EU, 0x90U, 0xFFU, 0xFFU } },
    { U"firebrick", { 0xB2U, 0x22U, 0x22U, 0xFFU } },
    { U"floralwhite", { 0xFFU, 0xFAU, 0xF0U, 0xFFU } },
    { U"forestgreen", { 0x22U, 0x8BU, 0x22U, 0xFFU } },
    { U"fuchsia", { 0xFFU, 0x00U, 0xFFU, 0xFFU } },
    { U"gainsboro", { 0xDCU, 0xDCU, 0xDCU, 0xFFU } },
    { U"ghostwhite", { 0xF8U, 0xF8U, 0xFFU, 0xFFU } },
    { U"gold", { 0xFFU, 0xD7U, 0x00U, 0xFFU } },
    { U"goldenrod", { 0xDAU, 0xA5U, 0x20U, 0xFFU } },
    { U"gray", { 0x80U, 0x80U, 0x80U, 0xFFU } },
    { U"green", { 0x00U, 0x80U, 0x00U, 0xFFU } },
    { U"greenyellow", { 0xADU, 0xFFU, 0x2FU, 0xFFU } },
    { U"grey", { 0x80U, 0x80U, 0x80U, 0xFFU } },
    { U"honeydew", { 0xF0U, 0xFFU, 0xF0U, 0xFFU } },
    { U"hotpink", { 0xFFU, 0x69U, 0xB4U, 0xFFU } },
    { U"indianred", { 0xCDU, 0x5CU, 0x5CU, 0xFFU } },
    { U"indigo", { 0x4BU, 0x00U, 0x82U, 0xFFU } },
    { U"ivory", { 0xFFU, 0xFFU, 0xF0U, 0xFFU } },
    { U"khaki", { 0xF0U, 0xE6U, 0x8CU, 0xFFU } },
    { U"lavender", { 0xE6U, 0xE6U, 0xFAU, 0xFFU } },
    { U"lavenderblush", { 0xFFU, 0xF0U, 0xF5U, 0xFFU } },
    { U"lawngreen", { 0x7CU, 0xFCU, 0x00U, 0xFFU } },
    { U"lemonchiffon", { 0xFFU, 0xFAU, 0xCDU, 0xFFU } },
    { U"lightblue", { 0xADU, 0xD8U, 0xE6U, 0xFFU } },
    { U"lightcoral", { 0xF0U, 0x80U, 0x80U, 0xFFU } },
    { U"lightcyan", { 0xE0U, 0xFFU, 0xFFU, 0xFFU } },
    { U"lightgoldenrodyellow", { 0xFAU, 0xFAU, 0xD2U, 0xFFU } },
    { U"lightgray", { 0xD3U, 0xD3U, 0xD3U, 0xFFU } },
    { U"lightgreen", { 0x90U, 0xEEU, 0x90U, 0xFFU } },
    { U"lightgrey", { 0xD3U, 0xD3U, 0xD3U, 0xFFU } },
    { U"lightpink", { 0xFFU, 0xB6U, 0xC1U, 0xFFU } },
    { U"lightsalmon", { 0xFFU, 0xA0U, 0x7AU, 0xFFU } },
    { U"lightseagreen", { 0x20U, 0xB2U, 0xAAU, 0xFFU } },
    { U"lightskyblue", { 0x87U, 0xCEU, 0xFAU, 0xFFU } },
    { U"lightslategray", { 0x77U, 0x88U, 0x99U, 0xFFU } },
    { U"lightslategrey", { 0x77U, 0x88U, 0x99U, 0xFFU } },
    { U"lightsteelblue", { 0xB0U, 0xC4U, 0xDEU, 0xFFU } },
    { U"lightyellow", { 0xFFU, 0xFFU, 0xE0U, 0xFFU } },
    { U"lime", { 0x00U, 0xFFU, 0x00U, 0xFFU } },
    { U"limegreen", { 0x32U, 0xCDU, 0x32U, 0xFFU } },
    { U"linen", { 0xFAU, 0xF0U, 0xE6U, 0xFFU } },
    { U"magenta", { 0xFFU, 0x00U, 0xFFU, 0xFFU } },
    { U"maroon", { 0x80U, 0x00U, 0x00U, 0xFFU } },
    { U"mediumaquamarine", { 0x66U, 0xCDU, 0xAAU, 0xFFU } },
    { U"mediumblue", { 0x00U, 0x00U, 0xCDU, 0xFFU } },
    { U"mediumorchid", { 0xBAU, 0x55U, 0xD3U, 0xFFU } },
    { U"mediumpurple", { 0x93U, 0x70U, 0xDBU, 0xFFU } },
    { U"mediumseagreen", { 0x3CU, 0xB3U, 0x71U, 0xFFU } },
    { U"mediumslateblue", { 0x7BU, 0x68U, 0xEEU, 0xFFU } },
    { U"mediumspringgreen", { 0x00U, 0xFAU, 0x9AU, 0xFFU } },
    { U"mediumturquoise", { 0x48U, 0xD1U, 0xCCU, 0xFFU } },
    { U"mediumvioletred", { 0xC7U, 0x15U, 0x85U, 0xFFU } },
    { U"midnightblue", { 0x19U, 0x19U, 0x70U, 0xFFU } },
    { U"mintcream", { 0xF5U, 0xFFU, 0xFAU, 0xFFU } },
    { U"mistyrose", { 0xFFU, 0xE4U, 0xE1U, 0xFFU } },
    { U"moccasin", { 0xFFU, 0xE4U, 0xB5U, 0xFFU } },
    { U"navajowhite", { 0xFFU, 0xDEU, 0xADU, 0xFFU } },
    { U"navy", { 0x00U, 0x00U, 0x80U, 0xFFU } },
    { U"oldlace", { 0xFDU, 0xF5U, 0xE6U, 0xFFU } },
    { U"olive", { 0x80U, 0x80U, 0x00U, 0xFFU } },
    { U"olivedrab", { 0x6BU, 0x8EU, 0x23U, 0xFFU } },
    { U"orange", { 0xFFU, 0xA5U, 0x00U, 0xFFU } },
    { U"orangered", { 0xFFU, 0x45U, 0x00U, 0xFFU } },
    { U"orchid", { 0xDAU, 0x70U, 0xD6U, 0xFFU } },
    { U"palegoldenrod", { 0xEEU, 0xE8U, 0xAAU, 0xFFU } },
    { U"palegreen", { 0x98U, 0xFBU, 0x98U, 0xFFU } },
    { U"paleturquoise", { 0xAFU, 0xEEU, 0xEEU, 0xFFU } },
    { U"palevioletred", { 0xDBU, 0x70U, 0x93U, 0xFFU } },
    { U"papayawhip", { 0xFFU, 0xEFU, 0xD5U, 0xFFU } },
    { U"peachpuff", { 0xFFU, 0xDAU, 0xB9U, 0xFFU } },
    { U"peru", { 0xCDU, 0x85U, 0x3FU, 0xFFU } },
    { U"pink", { 0xFFU, 0xC0U, 0xCBU, 0xFFU } },
    { U"plum", { 0xDDU, 0xA0U, 0xDDU, 0xFFU } },
    { U"powderblue", { 0xB0U, 0xE0U, 0xE6U, 0xFFU } },
    { U"purple", { 0x80U, 0x00U, 0x80U, 0xFFU } },
    { U"rebeccapurple", { 0x66U, 0x33U, 0x99U, 0xFFU } },
    { U"red", { 0xFFU, 0x00U, 0x00U, 0xFFU } },
    { U"rosybrown", { 0xBCU, 0x8FU, 0x8FU, 0xFFU } },
    { U"royalblue", { 0x41U, 0x69U, 0xE1U, 0xFFU } },
    { U"saddlebrown", { 0x8BU, 0x45U, 0x13U, 0xFFU } },
    { U"salmon", { 0xFAU, 0x80U, 0x72U, 0xFFU } },
    { U"sandybrown", { 0xF4U, 0xA4U, 0x60U, 0xFFU } },
    { U"seagreen", { 0x2EU, 0x8BU, 0x57U, 0xFFU } },
    { U"seashell", { 0xFFU, 0xF5U, 0xEEU, 0xFFU } },
    { U"sienna", { 0xA0U, 0x52U, 0x2DU, 0xFFU } },
    { U"silver", { 0xC0U, 0xC0U, 0xC0U, 0xFFU } },
    { U"skyblue", { 0x87U, 0xCEU, 0xEBU, 0xFFU } },
    { U"slateblue", { 0x6AU, 0x5AU, 0xCDU, 0xFFU } },
    { U"slategray", { 0x70U, 0x80U, 0x90U, 0xFFU } },
    { U"slategrey", { 0x70U, 0x80U, 0x90U, 0xFFU } },
    { U"snow", { 0xFFU, 0xFAU, 0xFAU, 0xFFU } },
    { U"springgreen", { 0x00U, 0xFFU, 0x7FU, 0xFFU } },
    { U"steelblue", { 0x46U, 0x82U, 0xB4U, 0xFFU } },
    { U"tan", { 0xD2U, 0xB4U, 0x8CU, 0xFFU } },
    { U"teal", { 0x00U, 0x80U, 0x80U, 0xFFU } },
    { U"thistle", { 0xD8U, 0xBFU, 0xD8U, 0xFFU } },
    { U"tomato", { 0xFFU, 0x63U, 0x47U, 0xFFU } },
    { U"transparent", { 0x00U, 0x00U, 0x00U, 0xFFU } },
    { U"turquoise", { 0x40U, 0xE0U, 0xD0U, 0xFFU } },
    { U"violet", { 0xEEU, 0x82U, 0xEEU, 0xFFU } },
    { U"wheat", { 0xF5U, 0xDEU, 0xB3U, 0xFFU } },
    { U"white", { 0xFFU, 0xFFU, 0xFFU, 0xFFU } },
    { U"whitesmoke", { 0xF5U, 0xF5U, 0xF5U, 0xFFU } },
    { U"yellow", { 0xFFU, 0xFFU, 0x00U, 0xFFU } },
    { U"yellowgreen", { 0x9AU, 0xCDU, 0x32U, 0xFFU } }
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
        _target = ColorValue ( true, GXColorUNORM ( 0U, 0U, 0U, 0U ) );
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
        _target = ColorValue ( false,
            GXColorUNORM ( ToComponent ( r ), ToComponent ( g ), ToComponent ( b ), OPAQUE_VALUE )
        );

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
            GXColorUNORM ( ToComponent ( r ), ToComponent ( g ), ToComponent ( b ), ToComponent ( a ) )
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
            GXColorUNORM ( ToComponent ( rH, rL ), ToComponent ( gH, gL ), ToComponent ( bH, bL ), OPAQUE_VALUE )
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
            GXColorUNORM ( ToComponent ( rH, rL ),
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

    GXColorRGB color {};
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
        color._data[ 3U ] = 1.0F;
        _target = ColorValue ( false, color.ToColorUNORM () );
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
        _target = ColorValue ( false, color.ToColorUNORM () );
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

    GXColorRGB color {};
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
        color._data[ 3U ] = 1.0F;
        _target = ColorValue ( false, color.ToColorUNORM () );
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
        _target = ColorValue ( false, color.ToColorUNORM () );
        return true;
    }

    android_vulkan::LogError ( "pbr::ColorPropertyChecker::HandleRGBColor - %s:%zu: Expected 4 component color value.",
        _css,
        line
    );

    return std::nullopt;
}

bool ColorPropertyChecker::Convert ( GXColorRGB &color,
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

uint8_t ColorPropertyChecker::ToComponent ( char32_t c ) noexcept
{
    constexpr uint8_t const lut[] =
    {
        0U,
        17U,
        34U,
        51U,
        68U,
        85U,
        102U,
        119U,
        136U,
        153U,
        170U,
        187U,
        204U,
        221U,
        238U,
        255U
    };

    return lut[ FromHEX ( c ) ];
}

uint8_t ColorPropertyChecker::ToComponent ( char32_t high, char32_t low ) noexcept
{
    return static_cast<uint8_t> ( FromHEX ( low ) | ( FromHEX ( high ) << UINT8_C ( 4 ) ) );
}

} // namespace pbr
