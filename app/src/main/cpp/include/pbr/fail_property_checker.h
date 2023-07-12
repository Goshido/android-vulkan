#ifndef PBR_FAIL_PROPERTY_CHECKER_H
#define PBR_FAIL_PROPERTY_CHECKER_H


#include "property_checker.h"


namespace pbr {

class FailPropertyChecker final : public PropertyChecker
{
    public:
        FailPropertyChecker () = delete;

        FailPropertyChecker ( FailPropertyChecker const & ) = delete;
        FailPropertyChecker &operator = ( FailPropertyChecker const & ) = delete;

        FailPropertyChecker ( FailPropertyChecker && ) = delete;
        FailPropertyChecker &operator = ( FailPropertyChecker && ) = delete;

        explicit FailPropertyChecker ( char const* css, Property::eType property ) noexcept;

        ~FailPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_FAIL_PROPERTY_CHECKER_H
