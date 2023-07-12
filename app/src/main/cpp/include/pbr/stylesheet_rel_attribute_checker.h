#ifndef PBR_STYLESHEET_REL_ATTRIBUTE_CHECKER_HPP
#define PBR_STYLESHEET_REL_ATTRIBUTE_CHECKER_HPP


#include "attribute_checker.h"


namespace pbr {

class StylesheetRELAttributeChecker final : public AttributeChecker
{
    public:
        StylesheetRELAttributeChecker () = delete;

        StylesheetRELAttributeChecker ( StylesheetRELAttributeChecker const & ) = delete;
        StylesheetRELAttributeChecker &operator = ( StylesheetRELAttributeChecker const & ) = delete;

        StylesheetRELAttributeChecker ( StylesheetRELAttributeChecker && ) = delete;
        StylesheetRELAttributeChecker &operator = ( StylesheetRELAttributeChecker && ) = delete;

        explicit StylesheetRELAttributeChecker ( char const *html ) noexcept;

        ~StylesheetRELAttributeChecker () override = default;

        [[nodiscard]] Result Process ( AttributeParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_STYLESHEET_REL_ATTRIBUTE_CHECKER_HPP
