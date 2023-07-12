#ifndef PBR_COLOR_PROPERTY_CHECKER_HPP
#define PBR_COLOR_PROPERTY_CHECKER_HPP


#include "color_value.h"
#include "number_parser.h"
#include "property_checker.h"


namespace pbr {

class ColorPropertyChecker final : public PropertyChecker
{
    private:
        struct ParseInfo final
        {
            // Can't be empty.
            std::u32string_view                                         _value;

            // Can't be empty.
            std::u32string_view                                         _tail;
        };

        using ParseResult = std::optional<ParseInfo>;

    private:
        static std::unordered_map<std::u32string, GXColorRGB> const     _colorMap;
        ColorValue                                                      &_target;

    public:
        ColorPropertyChecker () = delete;

        ColorPropertyChecker ( ColorPropertyChecker const & ) = delete;
        ColorPropertyChecker &operator = ( ColorPropertyChecker const & ) = delete;

        ColorPropertyChecker ( ColorPropertyChecker && ) = delete;
        ColorPropertyChecker &operator = ( ColorPropertyChecker && ) = delete;

        explicit ColorPropertyChecker ( char const* css, Property::eType property, ColorValue &target ) noexcept;

        ~ColorPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;

    private:
        [[nodiscard]] Result HandleHEXColor ( std::u32string_view value, size_t line ) noexcept;
        [[nodiscard]] Result HandleHEXColor3 ( std::u32string_view value, size_t line ) noexcept;
        [[nodiscard]] Result HandleHEXColor4 ( std::u32string_view value, size_t line ) noexcept;
        [[nodiscard]] Result HandleHEXColor6 ( std::u32string_view value, size_t line ) noexcept;
        [[nodiscard]] Result HandleHEXColor8 ( std::u32string_view value, size_t line ) noexcept;

        [[nodiscard]] Result HandleHSLColor ( std::u32string_view value, size_t line ) noexcept;
        [[nodiscard]] Result HandleRGBColor ( std::u32string_view value, size_t line ) noexcept;

        [[nodiscard]] bool Convert ( GXVec4 &color,
            std::optional<NumberParser::Result> const &number,
            size_t line,
            size_t component,
            float pureMin,
            float pureMax,
            float pureScale,
            float percentScale
        ) const noexcept;

        [[nodiscard]] static uint8_t FromHEX ( char32_t c ) noexcept;
        [[nodiscard]] static bool IsHEX ( char32_t c ) noexcept;
        [[nodiscard]] ParseResult ParseParameter ( size_t line, std::u32string_view value ) noexcept;
        [[nodiscard]] static float ToComponent ( char32_t c ) noexcept;
        [[nodiscard]] static float ToComponent ( char32_t high, char32_t low ) noexcept;
};

} // namespace pbr


#endif // PBR_COLOR_PROPERTY_CHECKER_HPP
