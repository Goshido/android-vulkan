#ifndef PBR_LENGTH_PROPERTY_CHECKER_HPP
#define PBR_LENGTH_PROPERTY_CHECKER_HPP


#include "length_value.hpp"
#include "property_checker.hpp"


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


#endif // PBR_LENGTH_PROPERTY_CHECKER_HPP
