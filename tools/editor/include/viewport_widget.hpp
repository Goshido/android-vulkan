#ifndef EDITOR_VIEWPORT_WIDGET_HPP
#define EDITOR_VIEWPORT_WIDGET_HPP


#include "div_ui_element.hpp"
#include "hotkey.hpp"
#include "move_tool.hpp"
#include "rotate_tool.hpp"
#include "scale_tool.hpp"
#include "select_tool.hpp"
#include "selection.hpp"
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
            uint8_t                         _forward: 1 = 0U;
            uint8_t                         _backward: 1 = 0U;
            uint8_t                         _left: 1 = 0U;
            uint8_t                         _right: 1 = 0U;
            uint8_t                         _ctrl: 1 = 0U;
            uint8_t                         _shift: 1 = 0U;
            uint8_t                         _alt: 1 = 0U;
            uint8_t                         _middleMouseButton: 1 = 0U;
            uint8_t                         _leftMouseButton: 1 = 0U;
        };

    private:
        DIVUIElement                        _div;

        DIVUIElement                        _selectionBody;
        DIVUIElement                        _selectionTop;
        DIVUIElement                        _selectionRight;
        DIVUIElement                        _selectionBottom;
        DIVUIElement                        _selectionLeft;

        MoveTool                            _moveTool {};
        Hotkey                              _useMoveTool {};

        RotateTool                          _rotateTool {};
        Hotkey                              _useRotateTool {};

        ScaleTool                           _scaleTool {};
        Hotkey                              _useScaleTool {};

        SelectTool                          _selectTool {};
        Hotkey                              _useSelectTool {};

        Tool*                               _activeTool = nullptr;

        GXMat4                              _projection = GXMat4::IDENTITY;
        VkExtent2D                          _resolution {};
        std::vector<float>                  _lineHeights = { 0.0F };
        GXQuat                              _orientation = GXQuat::IDENTITY;
        GXVec3                              _location = GXVec3::ZERO;

        VkOffset2D                          _mouseNow {};
        VkOffset2D                          _mouseCommit {};
        size_t                              _eventID = 0U;
        GXVec2                              _eulerAngles = GXVec2::ZERO;

        std::optional<Selection::eMode>     _selectionMode = std::nullopt;
        bool                                _selectionDrag = false;

        eNavigationMode                     _navigationMode = eNavigationMode::None;
        State                               _state {};

    public:
        explicit ViewportWidget () noexcept;

        ViewportWidget ( ViewportWidget const & ) = delete;
        ViewportWidget &operator = ( ViewportWidget const & ) = delete;

        ViewportWidget ( ViewportWidget && ) = delete;
        ViewportWidget &operator = ( ViewportWidget && ) = delete;

        ~ViewportWidget () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        void Update ( float deltaTime, float dpi ) noexcept;

        [[nodiscard]] GXMat4 const &GetProjection () const noexcept;
        [[nodiscard]] GXQuat const &GetOrientation () const noexcept;
        [[nodiscard]] GXVec3 const &GetLocation () const noexcept;

        // See <repo>/docs/gizmo-rendering.md#pixel-coverage
        [[nodiscard]] GXVec3 GetVI ( float invHeight ) const noexcept;

    private:
        void OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept override;
        void OnKeyboardKeyUp ( eKey key, KeyModifier modifier ) noexcept override;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

        [[nodiscard]] LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
            pbr::FontStorage &fontStorage
        ) noexcept override;

        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept override;

        void UpdateKeyboardState ( eKey key, KeyModifier modifier, uint8_t matchValue ) noexcept;
        void UpdateMouseState ( MouseButtonEvent const &event, uint8_t matchValue ) noexcept;
        void UpdateSelection ( int32_t left, int32_t top, int32_t width, int32_t height ) noexcept;
        void UpdateSelectionMode () noexcept;
        void ResolveNavigationMode () noexcept;

        void DoFreeFly ( float deltaTime, float dpi ) noexcept;
        void DoOrbit () noexcept;

        void SwitchTool ( Tool &tool ) noexcept;
};

} // namespace editor


#endif // EDITOR_VIEWPORT_WIDGET_HPP
