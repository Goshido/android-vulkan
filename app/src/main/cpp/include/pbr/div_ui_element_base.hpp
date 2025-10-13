#ifndef PBR_DIV_UI_ELEMENT_BASE_HPP
#define PBR_DIV_UI_ELEMENT_BASE_HPP


#include "css_computed_values.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace pbr {

template<typename E, typename S0, typename S1, typename U>
class DIVUIElementBase final : public E
{
    public:
        struct Rect final
        {
            GXVec2              _topLeft {};
            GXVec2              _bottomRight {};
        };

    private:
        Rect                    _absoluteRect {};

        GXVec2                  _blockSize {};
        GXVec2                  _borderSize {};

        GXVec2                  _canvasSize {};
        GXVec2                  _canvasTopLeftOffset {};

        std::deque<E*>          _children {};
        bool                    _hasBackground = false;
        bool                    _hasChanges = false;
        GXVec2                  _marginTopLeft {};

        std::vector<float>      _lineHeights {};
        size_t                  _parentLine = 0U;

        S0                      _stream0[ U::GetVerticesPerRectangle() ] {};
        S1                      _stream1[ U::GetVerticesPerRectangle() ] {};

    public:
        DIVUIElementBase () = delete;

        DIVUIElementBase ( DIVUIElementBase const & ) = delete;
        DIVUIElementBase &operator = ( DIVUIElementBase const & ) = delete;

        DIVUIElementBase ( DIVUIElementBase && ) = default;
        DIVUIElementBase &operator = ( DIVUIElementBase && ) = delete;

        explicit DIVUIElementBase ( E const* parent, CSSComputedValues &&css ) noexcept;
        explicit DIVUIElementBase ( E const* parent, CSSComputedValues &&css, std::string &&name ) noexcept;

        ~DIVUIElementBase () override = default;

        void ApplyLayout ( E::ApplyInfo &info ) noexcept override;
        void Submit ( E::SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( E::UpdateInfo &info ) noexcept override;

        void AppendChildElement ( E &element ) noexcept;
        void PrependChildElement ( E &element ) noexcept;

        [[nodiscard]] Rect const &GetAbsoluteRect () const noexcept;
        void Update () noexcept;
};

} // namespace pbr


#include <pbr/div_ui_element_base.ipp>


#endif // PBR_DIV_UI_ELEMENT_BASE_HPP
