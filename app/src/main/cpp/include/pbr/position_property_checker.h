#ifndef PBR_POSITION_PROPERTY_CHECKER_H
#define PBR_POSITION_PROPERTY_CHECKER_H


#include "position_property.h"
#include "property_checker.h"


namespace pbr {

class PositionPropertyChecker final : public PropertyChecker
{
    private:
        PositionProperty::eValue&       _target;

    public:
        PositionPropertyChecker () = delete;

        PositionPropertyChecker ( PositionPropertyChecker const & ) = delete;
        PositionPropertyChecker& operator = ( PositionPropertyChecker const & ) = delete;

        PositionPropertyChecker ( PositionPropertyChecker && ) = delete;
        PositionPropertyChecker& operator = ( PositionPropertyChecker && ) = delete;

        explicit PositionPropertyChecker ( char const* css, PositionProperty::eValue &target ) noexcept;

        ~PositionPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_POSITION_PROPERTY_CHECKER_H
