#include <precompiled_headers.hpp>
#include <pbr/color_property.hpp>
#include <pbr/color_property_checker.hpp>
#include <pbr/common_css_rule.hpp>
#include <pbr/display_property_checker.hpp>
#include <pbr/fail_property_checker.hpp>
#include <pbr/font_family_property.hpp>
#include <pbr/font_family_property_checker.hpp>
#include <pbr/length_property.hpp>
#include <pbr/length_property_checker.hpp>
#include <pbr/length_shorthand_property_checker.hpp>
#include <pbr/position_property.hpp>
#include <pbr/position_property_checker.hpp>
#include <pbr/text_align_property_checker.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/vertical_align_property_checker.hpp>
#include <pbr/whitespace.hpp>
#include <av_assert.hpp>
#include <logger.hpp>


namespace pbr {

ParseResult CommonCSSRule::Parse ( char const* css, Stream stream, eType type, CSSRules &cssRules ) noexcept
{
    size_t const l = stream._line;
    std::u32string name {};

    for ( ; ; )
    {
        auto const probe = UTF8Parser::Parse ( "pbr::CommonCSSRule::Parse", stream );

        if ( !probe )
            return std::nullopt;

        char32_t const c = probe->_character;

        if ( Whitespace::IsWhitespace ( c ) | ( c == U'{' ) )
            break;

        name.push_back ( c );
        stream = probe->_newStream;
    }

    constexpr auto resolveType = [] ( eType type ) noexcept -> char const* {
        constexpr char const* cases[] = { "Class", "ID" };
        return cases[ static_cast<size_t> ( type == eType::ID ) ];
    };

    if ( name.empty () )
    {
        android_vulkan::LogError ( "pbr::CommonCSSRule::Parse - %s:%zu: %s rule has empty name.",
            css,
            l,
            resolveType ( type )
        );

        return std::nullopt;
    }

    if ( cssRules.count ( name ) > 0 )
    {
        auto str = UTF8Parser::ToUTF8 ( name );
        std::string value {};

        if ( str )
        {
            value = std::move ( *str );
        }
        else
        {
            android_vulkan::LogWarning ( "pbr::CommonCSSRule::Parse - Can't convert to UTF-8." );
            AV_ASSERT ( false )
        }

        android_vulkan::LogError ( "pbr::CommonCSSRule::Parse - %s:%zu: %s rule with name '%s' already exist.",
            css,
            l,
            resolveType ( type ),
            value.c_str ()
        );

        return std::nullopt;
    }

    auto skip = Whitespace::Skip ( "pbr::CommonCSSRule::Parse", stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;
    auto probe = UTF8Parser::Parse ( "pbr::CommonCSSRule::Parse", stream );

    if ( !probe )
        return std::nullopt;

    char32_t const c = probe->_character;

    if ( c != U'{' )
    {
        android_vulkan::LogError ( "pbr::CommonCSSRule::Parse - %s:%zu: Expected '{'.", css, stream._line );
        return std::nullopt;
    }

    stream = probe->_newStream;

    if ( !stream.ExpectNotEmpty ( css, "pbr::CommonCSSRule::Parse" ) )
        return std::nullopt;

    PropertyChecker* checkers[ Property::GetTypeCount () ];

    LengthValue paddingBottom {};
    LengthValue paddingTop {};
    LengthValue paddingLeft {};
    LengthValue paddingRight {};

    LengthShorthandPropertyChecker paddingChecker ( css,
        Property::eType::Padding,
        paddingTop,
        paddingRight,
        paddingBottom,
        paddingLeft
    );

    checkers[ static_cast<size_t> ( Property::eType::Padding ) ] = &paddingChecker;

    LengthPropertyChecker paddingBottomChecker ( css, Property::eType::PaddingBottom, paddingBottom );
    checkers[ static_cast<size_t> ( Property::eType::PaddingBottom ) ] = &paddingBottomChecker;

    LengthPropertyChecker paddingTopChecker ( css, Property::eType::PaddingTop, paddingTop );
    checkers[ static_cast<size_t> ( Property::eType::PaddingTop ) ] = &paddingTopChecker;

    LengthPropertyChecker paddingLeftChecker ( css, Property::eType::PaddingLeft, paddingLeft );
    checkers[ static_cast<size_t> ( Property::eType::PaddingLeft ) ] = &paddingLeftChecker;

    LengthPropertyChecker paddingRightChecker ( css, Property::eType::PaddingRight, paddingRight );
    checkers[ static_cast<size_t> ( Property::eType::PaddingRight ) ] = &paddingRightChecker;

    LengthValue marginBottom {};
    LengthValue marginTop {};
    LengthValue marginLeft {};
    LengthValue marginRight {};

    LengthShorthandPropertyChecker marginChecker ( css,
        Property::eType::Margin,
        marginTop,
        marginRight,
        marginBottom,
        marginLeft
    );

    checkers[ static_cast<size_t> ( Property::eType::Margin ) ] = &marginChecker;

    LengthPropertyChecker marginBottomChecker ( css, Property::eType::MarginBottom, marginBottom );
    checkers[ static_cast<size_t> ( Property::eType::MarginBottom ) ] = &marginBottomChecker;

    LengthPropertyChecker marginTopChecker ( css, Property::eType::MarginTop, marginTop );
    checkers[ static_cast<size_t> ( Property::eType::MarginTop ) ] = &marginTopChecker;

    LengthPropertyChecker marginLeftChecker ( css, Property::eType::MarginLeft, marginLeft );
    checkers[ static_cast<size_t> ( Property::eType::MarginLeft ) ] = &marginLeftChecker;

    LengthPropertyChecker marginRightChecker ( css, Property::eType::MarginRight, marginRight );
    checkers[ static_cast<size_t> ( Property::eType::MarginRight ) ] = &marginRightChecker;

    LengthValue bottom {};
    LengthPropertyChecker bottomChecker ( css, Property::eType::Bottom, bottom );
    checkers[ static_cast<size_t> ( Property::eType::Bottom ) ] = &bottomChecker;

    LengthValue top {};
    LengthPropertyChecker topChecker ( css, Property::eType::Top, top );
    checkers[ static_cast<size_t> ( Property::eType::Top ) ] = &topChecker;

    LengthValue left {};
    LengthPropertyChecker leftChecker ( css, Property::eType::Left, left );
    checkers[ static_cast<size_t> ( Property::eType::Left ) ] = &leftChecker;

    LengthValue right {};
    LengthPropertyChecker rightChecker ( css, Property::eType::Right, right );
    checkers[ static_cast<size_t> ( Property::eType::Right ) ] = &rightChecker;

    LengthValue width {};
    LengthPropertyChecker widthChecker ( css, Property::eType::Width, width );
    checkers[ static_cast<size_t> ( Property::eType::Width ) ] = &widthChecker;

    LengthValue height {};
    LengthPropertyChecker heightChecker ( css, Property::eType::Height, height );
    checkers[ static_cast<size_t> ( Property::eType::Height ) ] = &heightChecker;

    std::u32string fontFamily {};
    FontFamilyPropertyChecker fontFamilyChecker ( css, fontFamily );
    checkers[ static_cast<size_t> ( Property::eType::FontFamily ) ] = &fontFamilyChecker;

    LengthValue fontSize {};
    LengthPropertyChecker fontSizeChecker ( css, Property::eType::FontSize, fontSize );
    checkers[ static_cast<size_t> ( Property::eType::FontSize ) ] = &fontSizeChecker;

    PositionProperty::eValue position;
    PositionPropertyChecker positionChecker ( css, position );
    checkers[ static_cast<size_t> ( Property::eType::Position ) ] = &positionChecker;

    ColorValue color {};
    ColorPropertyChecker colorChecker ( css, Property::eType::Color, color );
    checkers[ static_cast<size_t> ( Property::eType::Color ) ] = &colorChecker;

    ColorValue backgroundColor {};
    ColorPropertyChecker backgroundColorChecker ( css, Property::eType::BackgroundColor, backgroundColor );
    checkers[ static_cast<size_t> ( Property::eType::BackgroundColor ) ] = &backgroundColorChecker;

    LengthValue backgroundSize {};
    LengthPropertyChecker backgroundSizeChecker ( css, Property::eType::BackgroundSize, backgroundSize );
    checkers[ static_cast<size_t> ( Property::eType::BackgroundSize ) ] = &backgroundSizeChecker;

    FailPropertyChecker srcChecker ( css, Property::eType::SRC );
    checkers[ static_cast<size_t> ( Property::eType::SRC ) ] = &srcChecker;

    DisplayProperty::eValue display;
    DisplayPropertyChecker displayChecker ( css, display );
    checkers[ static_cast<size_t> ( Property::eType::Display ) ] = &displayChecker;

    VerticalAlignProperty::eValue verticalAlign;
    VerticalAlignPropertyChecker verticalAlignChecker ( css, verticalAlign );
    checkers[ static_cast<size_t> ( Property::eType::VerticalAlign ) ] = &verticalAlignChecker;

    TextAlignProperty::eValue textAlign;
    TextAlignPropertyChecker textAlignChecker ( css, textAlign );
    checkers[ static_cast<size_t> ( Property::eType::TextAlign ) ] = &textAlignChecker;

    bool hasProps = false;

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( css, "pbr::CommonCSSRule::Parse" ) )
            return std::nullopt;

        skip = Whitespace::Skip ( css, stream );

        if ( !skip )
            return std::nullopt;

        stream = *skip;
        probe = UTF8Parser::Parse ( css, stream );

        if ( !probe )
            return std::nullopt;

        if ( probe->_character == U'}' )
        {
            stream = probe->_newStream;
            break;
        }

        auto prop = PropertyParser::Parse ( css, stream );

        if ( !prop )
            return std::nullopt;

        auto action = checkers[ static_cast<size_t> ( prop->_property ) ]->Process ( *prop );

        if ( !action )
            return std::nullopt;

        hasProps = true;
        stream = prop->_newStream;
    }

    if ( !hasProps )
    {
        android_vulkan::LogError ( "pbr::CommonCSSRule::Parse - %s:%zu: Rule has no properties rule.", css, l );
        return std::nullopt;
    }

    CSSProps props {};

    bool shorthand = paddingChecker.IsDetected ();

    if ( shorthand | paddingTopChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::PaddingTop, paddingTop ) );

    if ( shorthand | paddingRightChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::PaddingRight, paddingRight ) );

    if ( shorthand | paddingBottomChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::PaddingBottom, paddingBottom ) );

    if ( shorthand | paddingLeftChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::PaddingLeft, paddingLeft ) );

    shorthand = marginChecker.IsDetected ();

    if ( shorthand | marginTopChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::MarginTop, marginTop ) );

    if ( shorthand | marginRightChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::MarginRight, marginRight ) );

    if ( shorthand | marginBottomChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::MarginBottom, marginBottom ) );

    if ( shorthand | marginLeftChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::MarginLeft, marginLeft ) );

    if ( topChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::Top, top ) );

    if ( rightChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::Right, right ) );

    if ( bottomChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::Bottom, bottom ) );

    if ( leftChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::Left, left ) );

    if ( widthChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::Width, width ) );

    if ( heightChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::Height, height ) );

    if ( fontSizeChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::FontSize, fontSize ) );

    if ( backgroundSizeChecker.IsDetected () )
        props.emplace_back ( std::make_unique<LengthProperty> ( Property::eType::BackgroundSize, backgroundSize ) );

    if ( fontFamilyChecker.IsDetected () )
        props.emplace_back ( std::make_unique<FontFamilyProperty> ( std::move ( fontFamily ) ) );

    if ( positionChecker.IsDetected () )
        props.emplace_back ( std::make_unique<PositionProperty> ( position ) );

    if ( colorChecker.IsDetected () )
        props.emplace_back ( std::make_unique<ColorProperty> ( Property::eType::Color, color ) );

    if ( backgroundColorChecker.IsDetected () )
        props.emplace_back ( std::make_unique<ColorProperty> ( Property::eType::BackgroundColor, backgroundColor ) );

    if ( displayChecker.IsDetected () )
        props.emplace_back ( std::make_unique<DisplayProperty> ( display ) );

    if ( verticalAlignChecker.IsDetected () )
        props.emplace_back ( std::make_unique<VerticalAlignProperty> ( verticalAlign ) );

    if ( textAlignChecker.IsDetected () )
        props.emplace_back ( std::make_unique<TextAlignProperty> ( textAlign ) );

    cssRules.emplace ( std::move ( name ), std::move ( props ) );
    return stream;
}

} // namespace pbr
