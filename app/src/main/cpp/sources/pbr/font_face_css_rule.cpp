#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/font_face_css_rule.hpp>
#include <pbr/font_family_property_checker.hpp>
#include <pbr/property_parser.hpp>
#include <pbr/src_property_checker.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

ParseResult FontFaceCSSRule::Parse ( char const* css,
    Stream stream,
    std::string const &assetRoot,
    std::unordered_map<std::u32string, std::string> &fonts
) noexcept
{
    size_t const l = stream._line;
    constexpr std::string_view const ruleName = "font-face";
    size_t const size = ruleName.size ();

    if ( stream._data.size () < ruleName.size () )
    {
        android_vulkan::LogError ( "pbr::FontFaceCSSRule::Parse - %s:%zu: Unexpected end of stream.", css, l );
        return std::nullopt;
    }

    auto const start = stream._data.begin ();

    bool const equal = std::equal ( ruleName.cbegin (),
        ruleName.cend (),
        start,
        start + size,

        [] ( char left, uint8_t right ) noexcept -> bool {
            return static_cast<int> ( left ) == std::tolower ( static_cast<int> ( right ) );
        }
    );

    if ( !equal )
    {
        android_vulkan::LogError ( "pbr::FontFaceCSSRule::Parse - %s:%zu: Only '@font-face' rule is supported.",
            css,
            l
        );

        return std::nullopt;
    }

    stream._data = stream._data.subspan ( size );

    if ( !stream.ExpectNotEmpty ( css, "pbr::FontFaceCSSRule::Parse" ) )
        return std::nullopt;

    auto skip = Whitespace::Skip ( css, stream );

    if ( !skip )
        return std::nullopt;

    stream = *skip;
    auto probe = UTF8Parser::Parse ( css, stream );

    if ( !probe )
        return std::nullopt;

    if ( probe->_character != U'{' )
    {
        android_vulkan::LogError ( "pbr::FontFaceCSSRule::Parse - %s:%zu: '{' expected.", css, stream._line );
        return std::nullopt;
    }

    stream = probe->_newStream;

    std::u32string fontFace {};
    FontFamilyPropertyChecker fontFamilyChecker ( css, fontFace );

    std::string fontFile {};
    SRCPropertyChecker fontFileChecker ( css, fontFile, assetRoot );

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( css, "pbr::FontFaceCSSRule::Parse" ) )
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

        auto action = fontFamilyChecker.Process ( *prop );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = prop->_newStream;
            continue;
        }

        action = fontFileChecker.Process ( *prop );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = prop->_newStream;
            continue;
        }

        android_vulkan::LogError ( "pbr::FontFaceCSSRule::Parse - %s:%zu: Unsupported property '%s' was detected  for "
            "'font-face' rule.",
            css,
            stream._line,
            PropertyParser::ToString ( prop->_property )
        );

        return std::nullopt;
    }

    if ( !fontFamilyChecker.IsDetected () )
    {
        android_vulkan::LogError ( "pbr::FontFaceCSSRule::Parse - %s:%zu: Expected 'font-family' property in "
            "'@font-face' rule.",
            css,
            l
        );

        return std::nullopt;
    }

    if ( !fontFileChecker.IsDetected () )
    {
        android_vulkan::LogError ( "pbr::FontFaceCSSRule::Parse - %s:%zu: Expected 'src' property in "
            "'@font-face' rule.",
            css,
            l
        );

        return std::nullopt;
    }

    auto findResult = fonts.find ( fontFace );

    if ( findResult == fonts.end () )
    {
        fonts.emplace ( std::move ( fontFace ), std::move ( fontFile ) );
        return stream;
    }

    auto const name = UTF8Parser::ToUTF8 ( fontFace );

    if ( !name )
    {
        android_vulkan::LogError ( "pbr::FontFaceCSSRule::Parse - %s:%zu: Can't parse font face name.", css, l );
        return std::nullopt;
    }

    android_vulkan::LogWarning ( "pbr::FontFaceCSSRule::Parse - %s:%zu: Font override was detected for face '%s' "
        "with file '%s'.",
        css,
        l,
        name->c_str (),
        fontFile.c_str ()
    );

    findResult->second = std::move ( fontFile );
    return stream;
}

} // namespace pbr
