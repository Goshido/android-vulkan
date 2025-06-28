#ifndef PBR_LINE_HEIGHT_PROPERTY_CHECKER_HPP
#define PBR_LINE_HEIGHT_PROPERTY_CHECKER_HPP


#include "length_value.hpp"
#include "property_checker.hpp"


namespace pbr {

class LineHeightPropertyChecker final : public PropertyChecker
{
    private:
        LengthValue     &_target;

    public:
        LineHeightPropertyChecker () = delete;

        LineHeightPropertyChecker ( LineHeightPropertyChecker const & ) = delete;
        LineHeightPropertyChecker &operator = ( LineHeightPropertyChecker const & ) = delete;

        LineHeightPropertyChecker ( LineHeightPropertyChecker && ) = delete;
        LineHeightPropertyChecker &operator = ( LineHeightPropertyChecker && ) = delete;

        explicit LineHeightPropertyChecker ( char const* css,
            LengthValue &target
        ) noexcept;

        ~LineHeightPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_LINE_HEIGHT_PROPERTY_CHECKER_HPP
