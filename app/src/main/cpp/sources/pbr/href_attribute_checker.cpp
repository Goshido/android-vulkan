#include <file.h>
#include <logger.h>
#include <pbr/href_attribute_checker.h>
#include <pbr/utf8_parser.h>


namespace pbr {

HREFAttributeChecker::HREFAttributeChecker ( char const *html, std::string &target, char const* assetRoot ) noexcept:
    AttributeChecker ( html, eAttribute::HREF ),
    _assetRoot ( assetRoot ),
    _target ( target )
{
    // NOTHING
}

AttributeChecker::Result HREFAttributeChecker::Process ( AttributeParser::Result &result ) noexcept
{
    auto const baseCheck = AttributeChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    auto const asset = UTF8Parser::ToUTF8 ( result._value );

    if ( !asset )
        return std::nullopt;

    std::string path = std::string ( _assetRoot ) + '/' + *asset;

    if ( android_vulkan::File ( path ).IsExist () )
    {
        _target = std::move ( path );
        return true;
    }

    android_vulkan::LogError ( "pbr::HREFAttributeChecker::Process - %s:%zu: Can't find asset '%s'.",
        _html,
        result._newStream._line,
        asset->c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
