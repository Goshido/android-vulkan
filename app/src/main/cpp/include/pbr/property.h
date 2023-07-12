#ifndef PBR_PROPERTY_HPP
#define PBR_PROPERTY_HPP


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Property
{
    public:
        enum class eType : size_t
        {
            BackgroundColor = 0U,
            BackgroundSize,
            Bottom,
            Color,
            Display,
            FontFamily,
            FontSize,
            Height,
            Left,
            Margin,
            MarginBottom,
            MarginLeft,
            MarginRight,
            MarginTop,
            Padding,
            PaddingBottom,
            PaddingLeft,
            PaddingRight,
            PaddingTop,
            Position,
            Right,
            SRC,
            TextAlign,
            Top,
            VerticalAlign,
            Width
        };

    private:
        eType       _type;

    public:
        Property () = delete;

        Property ( Property const & ) = default;
        Property &operator = ( Property const & ) = default;

        Property ( Property && ) = default;
        Property &operator = ( Property && ) = default;

        virtual ~Property () = default;

        [[nodiscard]] eType GetType () const noexcept;

        [[nodiscard]] consteval static size_t GetTypeCount () noexcept
        {
            return static_cast<size_t> ( eType::Width ) + 1U;
        }

    protected:
        explicit Property ( eType type ) noexcept;
};

} // namespace pbr


#endif // PBR_PROPERTY_HPP
