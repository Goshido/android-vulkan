#ifndef EDITOR_RECT_HPP
#define EDITOR_RECT_HPP


#include <pbr/css_computed_values.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace editor {

class Rect final
{
    public:
        int32_t     _left = 0;
        int32_t     _right = 0;
        int32_t     _top = 0;
        int32_t     _bottom = 0;

    public:
        explicit Rect () = default;

        Rect ( Rect const & ) = default;
        Rect &operator = ( Rect const & ) = default;

        Rect ( Rect && ) = default;
        Rect &operator = ( Rect && ) = default;

        constexpr explicit Rect ( int32_t left, int32_t right, int32_t top, int32_t bottom ) noexcept:
            _left ( left ),
            _right ( right ),
            _top ( top ),
            _bottom ( bottom )
        {
            // NOTHING
        }

        ~Rect () = default;

        void From ( GXVec2 const &topLeft, GXVec2 const &bottomRight ) noexcept;

        [[nodiscard]] bool IsOverlapped ( int32_t x, int32_t y ) const noexcept;
        [[nodiscard]] Rect ApplyOffset ( int32_t x, int32_t y ) const noexcept;

        [[nodiscard]] GXVec2 GetTopLeft () const noexcept;
        [[nodiscard]] GXVec2 GetBottomRight () const noexcept;

        [[nodiscard]] int32_t GetWidth () const noexcept;
        [[nodiscard]] int32_t GetHeight () const noexcept;

        void Normalize () noexcept;
        void ToCSSBounds ( pbr::CSSComputedValues &css ) noexcept;
};

} // namespace editor


#endif // EDITOR_RECT_HPP
