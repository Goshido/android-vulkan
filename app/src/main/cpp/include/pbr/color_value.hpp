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
        bool                _inherit = true;
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

        constexpr explicit ColorValue ( GXColorUNORM sRGB ) noexcept:
            _inherit ( false ),
            _srgb ( sRGB )
        {
            // NOTHING
        }

        constexpr explicit ColorValue ( uint32_t rSRGB, uint32_t gSRGB, uint32_t bSRGB, uint32_t a ) noexcept:
            _inherit ( false ),
            _srgb ( rSRGB, gSRGB, bSRGB, a )
        {
            // NOTHING
        }

        ~ColorValue () = default;

        void AttachNotifier ( Context context, NotifyChanged notifier ) noexcept;
        [[nodiscard]] GXColorUNORM GetSRGB () const noexcept;
        [[nodiscard]] bool IsInherit () const noexcept;
};

} // namespace pbr


#endif // PBR_COLOR_VALUE_HPP
