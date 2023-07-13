#ifndef PBR_IMAGE_ATTRIBUTE_CHECKER_HPP
#define PBR_IMAGE_ATTRIBUTE_CHECKER_HPP


#include "attribute_checker.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class ImageAttributeChecker final : public AttributeChecker
{
    private:
        char const*     _assetRoot = nullptr;
        std::string     &_target;

    public:
        ImageAttributeChecker () = delete;

        ImageAttributeChecker ( ImageAttributeChecker const & ) = delete;
        ImageAttributeChecker &operator = ( ImageAttributeChecker const & ) = delete;

        ImageAttributeChecker ( ImageAttributeChecker && ) = delete;
        ImageAttributeChecker &operator = ( ImageAttributeChecker && ) = delete;

        explicit ImageAttributeChecker ( char const *html, std::string &target, char const* assetRoot ) noexcept;

        ~ImageAttributeChecker () override = default;

        [[nodiscard]] Result Process ( AttributeParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_IMAGE_ATTRIBUTE_CHECKER_HPP
