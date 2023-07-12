#ifndef PBR_LENGTH_PROPERTY_CHECKER_H
#define PBR_LENGTH_PROPERTY_CHECKER_H


#include "length_value.h"
#include "property_checker.h"


namespace pbr {

class LengthPropertyChecker final : public PropertyChecker
{
    private:
        LengthValue     &_target;

    public:
        LengthPropertyChecker () = delete;

        LengthPropertyChecker ( LengthPropertyChecker const & ) = delete;
        LengthPropertyChecker &operator = ( LengthPropertyChecker const & ) = delete;

        LengthPropertyChecker ( LengthPropertyChecker && ) = delete;
        LengthPropertyChecker &operator = ( LengthPropertyChecker && ) = delete;

        explicit LengthPropertyChecker ( char const* css,
            Property::eType property,
            LengthValue &target
        ) noexcept;

        ~LengthPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_LENGTH_PROPERTY_CHECKER_H
