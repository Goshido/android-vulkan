#ifndef PBR_COLOR_VALUE_HPP
#define PBR_COLOR_VALUE_HPP


#include <GXCommon/GXMath.hpp>


namespace pbr {

class ColorValue final
{
    public:
        using Context = void*;
        using NotifyChanged = void ( * ) ( Context context ) noexcept;

    private:
        bool                _inherit = false;
        GXColorRGB          _srgb {};
        Context             _context = nullptr;
        NotifyChanged       _notifyChanged = nullptr;

    public:
        explicit ColorValue () = default;

        ColorValue ( ColorValue const & ) = default;
        ColorValue &operator = ( ColorValue const & ) = default;

        ColorValue ( ColorValue && ) = default;
        ColorValue &operator = ( ColorValue &&other ) noexcept;
        ColorValue &operator = ( GXColorRGB const &srgb ) noexcept;

        explicit ColorValue ( bool inherit, GXColorRGB const &srgb ) noexcept;

        ~ColorValue () = default;

        void AttachNotifier ( Context context, NotifyChanged notifier ) noexcept;
        [[nodiscard]] GXColorRGB GetLinearColor () const noexcept;
        [[nodiscard]] GXColorRGB const &GetSRGB () const noexcept;
        [[nodiscard]] bool IsInherit () const noexcept;
};

} // namespace pbr


#endif // PBR_COLOR_VALUE_HPP
