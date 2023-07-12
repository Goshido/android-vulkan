#ifndef PBR_ATTRIBUTE_CHECKER_HPP
#define PBR_ATTRIBUTE_CHECKER_HPP


#include "attribute_parser.hpp"


namespace pbr {

class AttributeChecker
{
    public:
        // std::nullopt in case of error.
        // true in case of match and processing.
        // false in case of skip.
        using Result = std::optional<bool>;

    private:
        bool                _detected = false;

    protected:
        eAttribute const    _attribute;
        char const*         _html;

    public:
        AttributeChecker () = delete;

        AttributeChecker ( AttributeChecker const & ) = delete;
        AttributeChecker &operator = ( AttributeChecker const & ) = delete;

        AttributeChecker ( AttributeChecker && ) = delete;
        AttributeChecker &operator = ( AttributeChecker && ) = delete;

        [[nodiscard]] bool IsDetected () const noexcept;

    protected:
        explicit AttributeChecker ( char const* html, eAttribute attribute ) noexcept;
        virtual ~AttributeChecker () = default;

        // Child classed must call this method. true means that the 'result' should be checked by child implementation.
        [[nodiscard]] virtual Result Process ( AttributeParser::Result &result ) noexcept;
};

} // namespace pbr


#endif // PBR_ATTRIBUTE_CHECKER_HPP
