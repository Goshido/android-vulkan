#ifndef PBR_DIV_UI_ELEMENT_HPP
#define PBR_DIV_UI_ELEMENT_HPP


#include "ui_element.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace pbr {

class DIVUIElement final : public UIElement
{
    private:
        GXColorRGB                  _backgroundColor {};
        GXVec2                      _blockSize {};
        GXVec2                      _borderSize {};

        GXVec2                      _canvasSize {};
        GXVec2                      _canvasTopLeftOffset {};

        std::deque<UIElement*>      _children {};
        bool                        _hasBackground = false;
        GXVec2                      _marginTopLeft {};

        bool                        _isAutoWidth;
        bool                        _isAutoHeight;
        bool                        _isInlineBlock;

        std::vector<float>          _lineHeights {};
        size_t                      _parentLine = 0U;
        UIVertexInfo                _vertices[ UIPass::GetVerticesPerRectangle() ] {};
        size_t const                _widthSelectorBase;

    public:
        DIVUIElement () = delete;

        DIVUIElement ( DIVUIElement const & ) = delete;
        DIVUIElement &operator = ( DIVUIElement const & ) = delete;

        DIVUIElement ( DIVUIElement && ) = default;
        DIVUIElement &operator = ( DIVUIElement && ) = delete;

        explicit DIVUIElement ( UIElement const* parent, CSSComputedValues &&css ) noexcept;

        ~DIVUIElement () override = default;

        void ApplyLayout ( ApplyInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( UpdateInfo &info ) noexcept override;

        void AppendChildElement ( UIElement &element ) noexcept;

    private:
        static void OnBackgroundColorChanged ( ColorValue::Context context ) noexcept;
};

} // namespace pbr


#endif // PBR_DIV_UI_ELEMENT_HPP
