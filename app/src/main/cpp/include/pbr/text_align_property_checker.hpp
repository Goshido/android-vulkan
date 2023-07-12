#ifndef PBR_TEXT_ALIGN_PROPERTY_CHECKER_HPP
#define PBR_TEXT_ALIGN_PROPERTY_CHECKER_HPP


#include "property_checker.hpp"
#include "text_align_property.hpp"


namespace pbr {

class TextAlignPropertyChecker final : public PropertyChecker
{
    private:
        TextAlignProperty::eValue&      _target;

    public:
        TextAlignPropertyChecker () = delete;

        TextAlignPropertyChecker ( TextAlignPropertyChecker const & ) = delete;
        TextAlignPropertyChecker &operator = ( TextAlignPropertyChecker const & ) = delete;

        TextAlignPropertyChecker ( TextAlignPropertyChecker && ) = delete;
        TextAlignPropertyChecker &operator = ( TextAlignPropertyChecker && ) = delete;

        explicit TextAlignPropertyChecker ( char const* css, TextAlignProperty::eValue &target ) noexcept;

        ~TextAlignPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;
};

} // namespace pbr


#endif // PBR_TEXT_ALIGN_PROPERTY_CHECKER_HPP
