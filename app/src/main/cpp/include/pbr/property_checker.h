#ifndef PBR_PROPERTY_CHECKER_HPP
#define PBR_PROPERTY_CHECKER_HPP


#include "property_parser.h"


namespace pbr {

class PropertyChecker
{
    public:
        // std::nullopt in case of error.
        // true in case of match and processing.
        // false in case of skip.
        using Result = std::optional<bool>;

    private:
        bool                        _detected = false;

    protected:
        char const*                 _css;
        Property::eType const       _property;

    public:
        PropertyChecker () = delete;

        PropertyChecker ( PropertyChecker const & ) = delete;
        PropertyChecker &operator = ( PropertyChecker const & ) = delete;

        PropertyChecker ( PropertyChecker && ) = delete;
        PropertyChecker &operator = ( PropertyChecker && ) = delete;

        // Child classed must call this method. true means that the 'result' should be checked by child implementation.
        [[nodiscard]] virtual Result Process ( PropertyParser::Result &result ) noexcept;

        [[nodiscard]] bool IsDetected () const noexcept;

    protected:
        explicit PropertyChecker ( char const* css, Property::eType property ) noexcept;
        virtual ~PropertyChecker () = default;
};

} // namespace pbr


#endif // PBR_PROPERTY_CHECKER_HPP
