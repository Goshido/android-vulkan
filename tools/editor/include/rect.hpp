#ifndef EDITOR_RECT_HPP
#define EDITOR_RECT_HPP


#include <GXCommon/GXWarning.hpp>

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

        [[nodiscard]] bool IsOverlapped ( int32_t x, int32_t y ) const noexcept;
        [[nodiscard]] Rect ApplyOffset ( int32_t x, int32_t y ) const noexcept;

        [[nodiscard]] int32_t GetWidth () const noexcept;
        [[nodiscard]] int32_t GetHeight () const noexcept;

        void Normalize () noexcept;
};

} // namespace editor


#endif // EDITOR_RECT_HPP
