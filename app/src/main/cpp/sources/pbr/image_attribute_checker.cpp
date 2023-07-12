#include <file.hpp>
#include <logger.hpp>
#include <pbr/image_attribute_checker.hpp>
#include <pbr/utf8_parser.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <regex>

GX_RESTORE_WARNING_STATE


namespace pbr {

ImageAttributeChecker::ImageAttributeChecker ( char const* html, std::string &target, char const* assetRoot ) noexcept:
    AttributeChecker ( html, eAttribute::SRC ),
    _assetRoot ( assetRoot ),
    _target ( target )
{
    // NOTHING
}

AttributeChecker::Result ImageAttributeChecker::Process ( AttributeParser::Result &result ) noexcept
{
    auto const baseCheck = AttributeChecker::Process ( result );

    if ( !baseCheck || !baseCheck.value () )
        return baseCheck;

    auto const imageFile = UTF8Parser::ToUTF8 ( result._value );

    if ( !imageFile )
        return std::nullopt;

    std::regex const regex ( R"__(\.[^\.]+$)__" );
    std::string const probe = std::regex_replace ( *imageFile, regex, ".ktx" );

    if ( probe.empty () )
    {
        android_vulkan::LogError ( "pbr::ImageAttributeChecker::Process - %s:%zu: Incorrect image asset '%s'.",
            _html,
            result._newStream._line,
            imageFile->c_str ()
        );

        return std::nullopt;
    }

    std::string const root = std::string ( _assetRoot ) + '/';
    std::string path = root + probe;

    if ( android_vulkan::File ( path ).IsExist () )
    {
        _target = std::move ( path );
        return true;
    }

    std::string originalFile = root + *imageFile;

    if ( path == originalFile )
    {
        android_vulkan::LogError ( "pbr::ImageAttributeChecker::Process - %s:%zu: Image asset '%s' does not exist.",
            _html,
            result._newStream._line,
            imageFile->c_str ()
        );

        return std::nullopt;
    }

    if ( android_vulkan::File ( originalFile ).IsExist () )
    {
        _target = std::move ( originalFile );
        return true;
    }

    android_vulkan::LogError ( "pbr::ImageAttributeChecker::Process - %s:%zu: Can't find image asset '%s'.",
        _html,
        result._newStream._line,
        imageFile->c_str ()
    );

    return std::nullopt;
}

} // namespace pbr
