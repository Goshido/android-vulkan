#ifndef PBR_DIV_UI_ELEMENT_HPP
#define PBR_DIV_UI_ELEMENT_HPP


#include "css_ui_element.h"

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace pbr {

class DIVUIElement final : public CSSUIElement
{
    private:
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

        DIVUIElement ( DIVUIElement && ) = delete;
        DIVUIElement &operator = ( DIVUIElement && ) = delete;

        explicit DIVUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            CSSComputedValues &&css
        ) noexcept;

        ~DIVUIElement () override = default;

        void ApplyLayout ( ApplyInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( UpdateInfo &info ) noexcept override;

        // Lua stack must have the following configuration:
        //      stack[ -1 ] -> child element
        //      stack[ -2 ] -> parent element
        // At the end method will remove 'child element' from Lua stack.
        [[nodiscard]] bool AppendChildElement ( lua_State &vm,
            int errorHandlerIdx,
            int appendChildElementIdx,
            UIElement &element
        ) noexcept;
};

} // namespace pbr


#endif // PBR_DIV_UI_ELEMENT_HPP
