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
        enum class eCoordinates : uint8_t
        {
            Global = UINT8_C ( 0 ),
            Local = UINT8_C ( 1 )
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
            uint8_t                         _mmb: 1 = 0U;
            uint8_t                         _lmb: 1 = 0U;
            uint8_t                         _rmb: 1 = 0U;
            uint8_t                         _esc: 1 = 0U;
            uint8_t                         _toolHit: 1 = 0U;
        };

        using Handler = void ( ViewportWidget::* ) () noexcept;

        struct StateHandlers final
        {
            Handler                         _keyDown = &ViewportWidget::OnIdleKeyDown;
            Handler                         _keyUp = &ViewportWidget::OnNothing;
            Handler                         _mouseMove = &ViewportWidget::OnNothing;
            Handler                         _stateEnter = &ViewportWidget::OnNothing;
        };

    private:
        StateHandlers                       _stateHandlers {};

        // FUCK - load this from editor save, last used tool.
        Handler                             _toolMouseMove = &ViewportWidget::OnRotateToolMouseMove;

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

        Hotkey                              _toggleCoordinates {};

        // FUCK - load this from editor save, last used tool.
        Tool*                               _activeTool = &_rotateTool;

        GXMat4                              _local = GXMat4::IDENTITY;
        GXMat4                              _projection = GXMat4::IDENTITY;
        GXMat4                              _viewProjection = GXMat4::IDENTITY;
        uint64_t                            _toView = 0U;

        VkExtent2D                          _resolution {};
        std::vector<float>                  _lineHeights = { 0.0F };

        GXQuat                              _rotation = GXQuat::IDENTITY;
        GXVec3                              _location = GXVec3::ZERO;
        float                               _invHeight = 1.0F;

        VkOffset2D                          _mouseNow {};
        VkOffset2D                          _mouseCommit {};
        size_t                              _eventID = 0U;
        GXVec2                              _eulerAngles = GXVec2::ZERO;

        Selection::eMode                    _selectionMode = Selection::eMode::Standby;
        bool                                _selectionDrag = false;

        State                               _state {};
        bool                                _toolVisible = false;

        // FUCK - load this from editor save, last used coordinates.
        eCoordinates                        _coordinates = eCoordinates::Local;

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

        [[nodiscard]] GXMat4 const &GetLocal () const noexcept;
        void SetLocal ( float yaw, float pitch, GXVec3 const &location ) noexcept;

        [[nodiscard]] uint64_t GetToView () const noexcept;
        [[nodiscard]] GXMat4 const &GetViewProjection () const noexcept;

        // See <repo>/docs/gizmo-rendering.md#pixel-coverage
        [[nodiscard]] GXVec3 GetVI () const noexcept;

        void OnSelectionChanged ( Selection::Items &items ) noexcept;

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

        [[nodiscard]] GXVec3 ComputeRayDirection ( GXMat3 const &basis ) const noexcept;
        void UpdateKeyboardState ( eKey key, KeyModifier modifier, uint8_t matchValue ) noexcept;
        void UpdateMouseState ( MouseButtonEvent const &event, uint8_t matchValue ) noexcept;
        void UpdateRotation () noexcept;
        void UpdateSelection ( int32_t left, int32_t top, int32_t width, int32_t height ) noexcept;
        void UpdateSelectionMode () noexcept;
        void UpdateToolCoordinates ( Selection::Items &items ) noexcept;
        void UpdateViewProjection () noexcept;

        void OnFreeFlyKeyUp () noexcept;
        void OnFreeFlyStateEnter () noexcept;

        void OnIdleKeyDown () noexcept;
        void OnIdleMouseMove () noexcept;
        void OnIdleStateEnter () noexcept;

        void OnSelectionKeyDown () noexcept;
        void OnSelectionKeyUp () noexcept;
        void OnSelectionMouseMove () noexcept;
        void OnSelectionStateEnter () noexcept;
        void StopSelection ( bool cancel ) noexcept;

        void OnToolKeyDown () noexcept;
        void OnToolKeyUp () noexcept;
        void OnToolMouseMove () noexcept;
        void OnToolStateEnter () noexcept;
        void StopTool () noexcept;

        void OnMoveToolMouseMove () noexcept;
        void OnRotateToolMouseMove () noexcept;
        void OnScaleToolMouseMove () noexcept;

        void OnNothing () noexcept;

        void DoFreeFly ( float deltaTime, float dpi ) noexcept;
        void SwitchTool ( Tool &tool ) noexcept;
};

} // namespace editor


#endif // EDITOR_VIEWPORT_WIDGET_HPP
