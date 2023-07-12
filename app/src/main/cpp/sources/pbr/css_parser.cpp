#include <logger.h>
#include <pbr/common_css_rule.h>
#include <pbr/css_parser.h>
#include <pbr/font_face_css_rule.h>
#include <pbr/utf8_parser.h>
#include <pbr/whitespace.h>


namespace pbr {

std::optional<CSSProps const*> CSSParser::FindClass ( std::u32string const &name ) const noexcept
{
    auto const findResult = _classes.find ( name );

    if ( findResult != _classes.cend () )
        return &findResult->second;

    return std::nullopt;
}

std::optional<std::string const*> CSSParser::FindFontFile ( std::u32string const &fontFamily ) const noexcept
{
    auto const findResult = _fonts.find ( fontFamily );

    if ( findResult != _fonts.cend () )
        return &findResult->second;

    return std::nullopt;
}

std::optional<CSSProps const*> CSSParser::FindID ( std::u32string const &id ) const noexcept
{
    auto const findResult = _ids.find ( id );

    if ( findResult != _ids.cend () )
        return &findResult->second;

    return std::nullopt;
}

std::string const &CSSParser::GetSource () const noexcept
{
    return _cssSource;
}

bool CSSParser::Parse ( char const* css, Stream stream, std::string &&assetRoot ) noexcept
{
    if ( !stream.ExpectNotEmpty ( css, "pbr::CSSParser::Parse" ) )
        return false;

    for ( ; ; )
    {
        auto skip = Whitespace::Skip ( css, stream );

        if ( !skip )
            return false;

        stream = *skip;

        if ( stream._data.empty () )
            break;

        auto const probe = UTF8Parser::Parse ( css, stream );

        if ( !probe )
            return false;

        char32_t const c = probe->_character;
        size_t const l = stream._line;
        stream = probe->_newStream;

        switch ( c )
        {
            case U'@':
            {
                ParseResult const face = FontFaceCSSRule::Parse ( css, stream, assetRoot, _fonts );

                if ( !face )
                    return false;

                stream = *face;
            }
            break;

            case U'.':
            {
                ParseResult const rule = CommonCSSRule::Parse ( css, stream,  CommonCSSRule::eType::Class, _classes );

                if ( !rule )
                    return false;

                stream = *rule;
            }
            break;

            case U'#':
            {
                ParseResult const rule = CommonCSSRule::Parse ( css, stream, CommonCSSRule::eType::ID, _ids );

                if ( !rule )
                    return false;

                stream = *rule;
            }
            break;

            default:
                android_vulkan::LogError ( "pbr::CSSParser::Parse - %s:%zu: Unexpected selector. It should be "
                    "started from '@', '.' or '#' character.",
                    css,
                    l
                );

            return false;
        }
    }

    _cssSource = css;
    return true;
}

} // namespace pbr
