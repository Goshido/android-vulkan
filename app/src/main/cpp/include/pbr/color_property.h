#ifndef PBR_COLOR_PROPERTY_H
#define PBR_COLOR_PROPERTY_H


#include "color_value.h"
#include "property.h"


namespace pbr {

class ColorProperty final : public Property
{
    private:
        ColorValue const    _value;

    public:
        ColorProperty () = delete;

        ColorProperty ( ColorProperty const & ) = delete;
        ColorProperty& operator = ( ColorProperty const & ) = delete;

        ColorProperty ( ColorProperty && ) = delete;
        ColorProperty& operator = ( ColorProperty && ) = delete;

        explicit ColorProperty ( eType type, ColorValue const &value ) noexcept;

        ~ColorProperty () override = default;

        [[nodiscard]] ColorValue const& GetValue () const noexcept;
};

} // namespace pbr


#endif // PBR_COLOR_PROPERTY_H
