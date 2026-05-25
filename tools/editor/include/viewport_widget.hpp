#ifndef EDITOR_VIEWPORT_WIDGET_HPP
#define EDITOR_VIEWPORT_WIDGET_HPP


#include "div_ui_element.hpp"
#include "widget.hpp"


namespace editor {

class ViewportWidget final : public Widget
{
    private:
        DIVUIElement    _div;
        VkExtent2D      _resolution {};
        float           _aspectRatio = 1.667F;

    public:
        explicit ViewportWidget () noexcept;

        ViewportWidget ( ViewportWidget const & ) = delete;
        ViewportWidget &operator = ( ViewportWidget const & ) = delete;

        ViewportWidget ( ViewportWidget && ) = delete;
        ViewportWidget &operator = ( ViewportWidget && ) = delete;

        ~ViewportWidget () = default;

    private:
        void OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept override;
        void OnKeyboardKeyUp ( eKey key, KeyModifier modifier ) noexcept override;

        void OnMouseLeave () noexcept override;
        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

        [[nodiscard]] bool UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept override;

        void UpdateCamera () noexcept;
};

} // namespace editor


#endif // EDITOR_VIEWPORT_WIDGET_HPP
