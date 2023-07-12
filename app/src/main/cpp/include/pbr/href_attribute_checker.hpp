#ifndef PBR_HREF_ATTRIBUTE_CHECKER_HPP
#define PBR_HREF_ATTRIBUTE_CHECKER_HPP


#include "attribute_checker.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class HREFAttributeChecker final : public AttributeChecker
{
    private:
        char const*     _assetRoot = nullptr;
        std::string     &_target;

    public:
        HREFAttributeChecker () = delete;

        HREFAttributeChecker ( HREFAttributeChecker const & ) = delete;
        HREFAttributeChecker &operator = ( HREFAttributeChecker const & ) = delete;

        HREFAttributeChecker ( HREFAttributeChecker && ) = delete;
        HREFAttributeChecker &operator = ( HREFAttributeChecker && ) = delete;

        explicit HREFAttributeChecker ( char const *html, std::string &target, char const* assetRoot ) noexcept;

        ~HREFAttributeChecker () override = default;

        [[nodiscard]] Result Process ( AttributeParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_HREF_ATTRIBUTE_CHECKER_HPP
