#ifndef PBR_SET_ATTRIBUTE_CHECKER_HPP
#define PBR_SET_ATTRIBUTE_CHECKER_HPP


#include "attribute_checker.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class SetAttributeChecker final : public AttributeChecker
{
    private:
        std::unordered_set<std::u32string>      _set {};
        std::unordered_set<std::u32string>      &_target;

    public:
        SetAttributeChecker () = delete;

        SetAttributeChecker ( SetAttributeChecker const & ) = delete;
        SetAttributeChecker &operator = ( SetAttributeChecker const & ) = delete;

        SetAttributeChecker ( SetAttributeChecker && ) = delete;
        SetAttributeChecker &operator = ( SetAttributeChecker && ) = delete;

        explicit SetAttributeChecker ( char const *html,
            eAttribute attribute,
            std::unordered_set<std::u32string> &target
        ) noexcept;

        ~SetAttributeChecker () override = default;

        [[nodiscard]] Result Process ( AttributeParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_SET_ATTRIBUTE_CHECKER_HPP
