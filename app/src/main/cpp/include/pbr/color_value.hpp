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
        GXColorUNORM        _srgb {};
        Context             _context = nullptr;
        NotifyChanged       _notifyChanged = nullptr;

    public:
        ColorValue () = default;

        ColorValue ( ColorValue const & ) = default;
        ColorValue &operator = ( ColorValue const & ) = default;

        ColorValue ( ColorValue && ) = default;
        ColorValue &operator = ( ColorValue &&other ) noexcept;
        ColorValue &operator = ( GXColorUNORM srgb ) noexcept;

        explicit ColorValue ( bool inherit, GXColorUNORM srgb ) noexcept;

        ~ColorValue () = default;

        void AttachNotifier ( Context context, NotifyChanged notifier ) noexcept;
        [[nodiscard]] GXColorUNORM GetSRGB () const noexcept;
        [[nodiscard]] bool IsInherit () const noexcept;
};

} // namespace pbr


#endif // PBR_COLOR_VALUE_HPP
