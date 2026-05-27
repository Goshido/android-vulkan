#ifndef EDITOR_VIEWPORT_WIDGET_HPP
#define EDITOR_VIEWPORT_WIDGET_HPP


#include "div_ui_element.hpp"
#include "widget.hpp"


namespace editor {

class ViewportWidget final : public Widget
{
    private:
        enum class eNavigationMode : uint8_t
        {
            FreeFly,
            Orbit,
            None
        };

        struct State final
        {
            uint8_t             _forward: 1U = 0U;
            uint8_t             _backward: 1U = 0U;
            uint8_t             _left: 1U = 0U;
            uint8_t             _right: 1U = 0U;
            uint8_t             _shift: 1U = 0U;
            uint8_t             _alt: 1U = 0U;
            uint8_t             _middleMouseButton: 1U = 0U;
            uint8_t             _leftMouseButton: 1U = 0U;
        };

        struct Mouse final
        {
            int32_t             _x = 0;
            int32_t             _y = 0;
        };

    private:
        DIVUIElement            _div;
        VkExtent2D              _resolution {};
        std::vector<float>      _lineHeights = { 0.0F };
        Mouse                   _mouseNow {};
        Mouse                   _mouseCommit {};
        size_t                  _eventID = 0U;
        float                   _aspectRatio = 1.667F;
        eNavigationMode         _navigationMode = eNavigationMode::None;
        State                   _state {};

    public:
        explicit ViewportWidget () noexcept;

        ViewportWidget ( ViewportWidget const & ) = delete;
        ViewportWidget &operator = ( ViewportWidget const & ) = delete;

        ViewportWidget ( ViewportWidget && ) = delete;
        ViewportWidget &operator = ( ViewportWidget && ) = delete;

        ~ViewportWidget () = default;

        void Update ( float deltaTime ) noexcept;

    private:
        void OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept override;
        void OnKeyboardKeyUp ( eKey key, KeyModifier modifier ) noexcept override;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

        [[nodiscard]] LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
            pbr::FontStorage &fontStorage
        ) noexcept override;

        void UpdateCamera () noexcept;
        void UpdateKeyboardState ( eKey key, KeyModifier modifier, uint8_t matchValue ) noexcept;
        void UpdateMouseState ( MouseButtonEvent const &event, uint8_t matchValue ) noexcept;
        void ResolveNavigationMode () noexcept;

        void DoFreeFly ( float deltaTime ) noexcept;
        void DoOrbit () noexcept;
};

} // namespace editor


#endif // EDITOR_VIEWPORT_WIDGET_HPP
