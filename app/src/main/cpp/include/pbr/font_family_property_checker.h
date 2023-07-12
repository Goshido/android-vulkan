#ifndef PBR_FONT_FAMILY_PROPERTY_CHECKER_HPP
#define PBR_FONT_FAMILY_PROPERTY_CHECKER_HPP


#include "property_checker.h"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class FontFamilyPropertyChecker final : public PropertyChecker
{
    private:
        std::u32string      &_target;

    public:
        FontFamilyPropertyChecker () = delete;

        FontFamilyPropertyChecker ( FontFamilyPropertyChecker const & ) = delete;
        FontFamilyPropertyChecker &operator = ( FontFamilyPropertyChecker const & ) = delete;

        FontFamilyPropertyChecker ( FontFamilyPropertyChecker && ) = delete;
        FontFamilyPropertyChecker &operator = ( FontFamilyPropertyChecker && ) = delete;

        explicit FontFamilyPropertyChecker ( char const* css, std::u32string &target ) noexcept;

        ~FontFamilyPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;

    private:
        [[nodiscard]] Result ParseQuoted ( size_t line, std::u32string_view value, char32_t quote ) noexcept;
        [[nodiscard]] Result ParseUnquoted ( size_t line, std::u32string_view value ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_FAMILY_PROPERTY_CHECKER_HPP
