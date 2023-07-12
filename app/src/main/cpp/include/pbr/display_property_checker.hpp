#ifndef PBR_DISPLAY_PROPERTY_CHECKER_HPP
#define PBR_DISPLAY_PROPERTY_CHECKER_HPP


#include "display_property.h"
#include "property_checker.h"


namespace pbr {

class DisplayPropertyChecker final : public PropertyChecker
{
    private:
        DisplayProperty::eValue     &_target;

    public:
        DisplayPropertyChecker () = delete;

        DisplayPropertyChecker ( DisplayPropertyChecker const & ) = delete;
        DisplayPropertyChecker &operator = ( DisplayPropertyChecker const & ) = delete;

        DisplayPropertyChecker ( DisplayPropertyChecker && ) = delete;
        DisplayPropertyChecker &operator = ( DisplayPropertyChecker && ) = delete;

        explicit DisplayPropertyChecker ( char const* css, DisplayProperty::eValue &target ) noexcept;

        ~DisplayPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_DISPLAY_PROPERTY_CHECKER_HPP
