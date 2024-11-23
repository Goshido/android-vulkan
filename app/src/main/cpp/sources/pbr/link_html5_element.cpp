#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/attribute_parser.hpp>
#include <pbr/href_attribute_checker.hpp>
#include <pbr/link_html5_element.hpp>
#include <pbr/stylesheet_rel_attribute_checker.hpp>
#include <pbr/whitespace.hpp>


namespace pbr {

std::optional<LinkHTML5Element::Result> LinkHTML5Element::Parse ( char const* html,
    Stream stream,
    const char* assetRoot
) noexcept
{
    size_t const l = stream._line;
    StylesheetRELAttributeChecker stylesheetChecker ( html );

    std::string css {};
    HREFAttributeChecker hrefChecker ( html, css, assetRoot );

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( html, "pbr::LinkHTML5Element::Parse" ) )
            return std::nullopt;

        auto const skip = Whitespace::Skip ( html, stream );

        if ( !skip )
            return std::nullopt;

        stream = *skip;
        auto att = AttributeParser::Parse ( html, stream );

        if ( !att )
            return std::nullopt;

        eAttribute const a = att->_attribute;

        if ( a == eAttribute::No_Attribute )
            break;

        auto action = stylesheetChecker.Process ( *att );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = att->_newStream;
            continue;
        }

        action = hrefChecker.Process ( *att );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = att->_newStream;
            continue;
        }

        android_vulkan::LogError ( "pbr::IMGHTML5Element::Parse - %s:%zu: Unsupported attribute detected '%s' for "
            "'link' element",
            html,
            stream._line,
            AttributeParser::ToString ( a )
        );

        return std::nullopt;
    }

    if ( !stylesheetChecker.IsDetected () )
    {
        android_vulkan::LogError ( "pbr::LinkHTML5Element::Process - %s:%zu: 'rel' attribute is not detected.",
            html,
            l
        );

        return std::nullopt;
    }

    if ( !hrefChecker.IsDetected () )
    {
        android_vulkan::LogError ( "pbr::LinkHTML5Element::Process - %s:%zu: 'href' attribute is not detected.",
            html,
            l
        );

        return std::nullopt;
    }

    if ( !stream.ExpectNotEmpty ( html, "pbr::LinkHTML5Element::Parse" ) )
        return std::nullopt;

    if ( stream._data.front () != static_cast<uint8_t> ( '>' ) )
    {
        android_vulkan::LogError ( "pbr::LinkHTML5Element::Parse - %s:%zu: 'link' element should be closed by '>'",
            html,
            stream._line
        );

        return std::nullopt;
    }

    std::string cssAssetRoot = assetRoot;
    auto const findResult = css.find_last_of ( '/' );
    size_t const s = cssAssetRoot.size ();

    if ( findResult != s )
        cssAssetRoot += css.substr ( s, findResult - s );

    return Result
    {
        ._assetRoot = std::move ( cssAssetRoot ),
        ._cssFile = std::move ( css ),
        ._newStream = Stream ( stream._data.subspan ( 1U ), stream._line )
    };
}

} // namespace pbr
