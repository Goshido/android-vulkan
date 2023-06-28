#ifndef PBR_LENGTH_SHORTHAND_PROPERTY_CHECKER_H
#define PBR_LENGTH_SHORTHAND_PROPERTY_CHECKER_H


#include "length_value.h"
#include "property_checker.h"


namespace pbr {

class LengthShorthandPropertyChecker final : public PropertyChecker
{
    private:
        LengthValue&    _bottom;
        LengthValue&    _top;
        LengthValue&    _left;
        LengthValue&    _right;

    public:
        LengthShorthandPropertyChecker () = delete;

        LengthShorthandPropertyChecker ( LengthShorthandPropertyChecker const & ) = delete;
        LengthShorthandPropertyChecker& operator = ( LengthShorthandPropertyChecker const & ) = delete;

        LengthShorthandPropertyChecker ( LengthShorthandPropertyChecker && ) = delete;
        LengthShorthandPropertyChecker& operator = ( LengthShorthandPropertyChecker && ) = delete;

        explicit LengthShorthandPropertyChecker ( char const* css,
            Property::eType property,
            LengthValue &top,
            LengthValue &right,
            LengthValue &bottom,
            LengthValue &left
        ) noexcept;

        ~LengthShorthandPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;

    private:
        [[nodiscard]] Result Process1Value ( std::u32string_view value, size_t line ) noexcept;
        [[nodiscard]] Result Process2Values ( std::list<std::u32string> const &value, size_t line ) noexcept;
        [[nodiscard]] Result Process3Values ( std::list<std::u32string> const &value, size_t line ) noexcept;
        [[nodiscard]] Result Process4Values ( std::list<std::u32string> const &value, size_t line ) noexcept;
};

} // namespace pbr


#endif // PBR_LENGTH_SHORTHAND_PROPERTY_CHECKER_H
