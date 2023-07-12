#ifndef PBR_VERTICAL_ALIGN_PROPERTY_CHECKER_HPP
#define PBR_VERTICAL_ALIGN_PROPERTY_CHECKER_HPP


#include "property_checker.h"
#include "vertical_align_property.h"


namespace pbr {

class VerticalAlignPropertyChecker final : public PropertyChecker
{
    private:
        VerticalAlignProperty::eValue       &_target;

    public:
        VerticalAlignPropertyChecker () = delete;

        VerticalAlignPropertyChecker ( VerticalAlignPropertyChecker const & ) = delete;
        VerticalAlignPropertyChecker &operator = ( VerticalAlignPropertyChecker const & ) = delete;

        VerticalAlignPropertyChecker ( VerticalAlignPropertyChecker && ) = delete;
        VerticalAlignPropertyChecker &operator = ( VerticalAlignPropertyChecker && ) = delete;

        explicit VerticalAlignPropertyChecker ( char const* css, VerticalAlignProperty::eValue &target ) noexcept;

        ~VerticalAlignPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_VERTICAL_ALIGN_PROPERTY_CHECKER_HPP
