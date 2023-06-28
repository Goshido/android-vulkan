#ifndef PBR_COLOR_VALUE_H
#define PBR_COLOR_VALUE_H


#include <GXCommon/GXMath.h>


namespace pbr {

class ColorValue final
{
    private:
        bool            _inherit;
        GXColorRGB      _value;

    public:
        ColorValue () = default;

        ColorValue ( ColorValue const & ) = default;
        ColorValue& operator = ( ColorValue const & ) = default;

        ColorValue ( ColorValue && ) = default;
        ColorValue& operator = ( ColorValue && ) = default;

        explicit ColorValue ( bool inherit, GXColorRGB const &value ) noexcept;

        ~ColorValue () = default;

        [[nodiscard]] GXColorRGB const& GetValue () const noexcept;
        [[nodiscard]] bool IsInherit () const noexcept;
};

} // namespace pbr


#endif // PBR_COLOR_VALUE_H
