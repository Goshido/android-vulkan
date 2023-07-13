#ifndef PBR_UNIQUE_ATTRIBUTE_CHECKER_HPP
#define PBR_UNIQUE_ATTRIBUTE_CHECKER_HPP


#include "attribute_checker.hpp"


namespace pbr {

class UniqueAttributeChecker final : public AttributeChecker
{
    private:
        std::unordered_set<std::u32string>      &_registry;
        std::u32string                          &_target;

    public:
        UniqueAttributeChecker () = delete;

        UniqueAttributeChecker ( UniqueAttributeChecker const & ) = delete;
        UniqueAttributeChecker &operator = ( UniqueAttributeChecker const & ) = delete;

        UniqueAttributeChecker ( UniqueAttributeChecker && ) = delete;
        UniqueAttributeChecker &operator = ( UniqueAttributeChecker && ) = delete;

        explicit UniqueAttributeChecker ( char const *html,
            eAttribute attribute,
            std::u32string &target,
            std::unordered_set<std::u32string> &registry
        ) noexcept;

        ~UniqueAttributeChecker () override = default;

        [[nodiscard]] Result Process ( AttributeParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_UNIQUE_ATTRIBUTE_CHECKER_HPP
