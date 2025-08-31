// FUCK - windows and android separation

#ifndef PBR_DIV_UI_ELEMENT_HPP
#define PBR_DIV_UI_ELEMENT_HPP


#include "ui_element.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace pbr {

class DIVUIElement final : public UIElement
{
    public:
        struct Rect final
        {
            GXVec2                  _topLeft {};
            GXVec2                  _bottomRight {};
        };

    private:
        Rect                        _absoluteRect {};

        GXVec2                      _blockSize {};
        GXVec2                      _borderSize {};

        GXVec2                      _canvasSize {};
        GXVec2                      _canvasTopLeftOffset {};

        std::deque<UIElement*>      _children {};
        bool                        _hasBackground = false;
        bool                        _hasChanges = false;
        GXVec2                      _marginTopLeft {};

        std::vector<float>          _lineHeights {};
        size_t                      _parentLine = 0U;

        // FUCK - remove namespace
        GXVec2                      _positions[ android::UIPass::GetVerticesPerRectangle() ] {};
        android::UIVertex           _vertices[ android::UIPass::GetVerticesPerRectangle() ] {};

    public:
        DIVUIElement () = delete;

        DIVUIElement ( DIVUIElement const & ) = delete;
        DIVUIElement &operator = ( DIVUIElement const & ) = delete;

        DIVUIElement ( DIVUIElement && ) = default;
        DIVUIElement &operator = ( DIVUIElement && ) = delete;

        explicit DIVUIElement ( UIElement const* parent, CSSComputedValues &&css ) noexcept;
        explicit DIVUIElement ( UIElement const* parent, CSSComputedValues &&css, std::string &&name ) noexcept;

        ~DIVUIElement () override = default;

        void ApplyLayout ( ApplyInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( UpdateInfo &info ) noexcept override;

        void AppendChildElement ( UIElement &element ) noexcept;
        void PrependChildElement ( UIElement &element ) noexcept;

        [[nodiscard]] Rect const &GetAbsoluteRect () const noexcept;
        void Update () noexcept;
};

} // namespace pbr


#endif // PBR_DIV_UI_ELEMENT_HPP
