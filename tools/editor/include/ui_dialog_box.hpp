#ifndef EDITOR_UI_DIALOG_BOX_HPP
#define EDITOR_UI_DIALOG_BOX_HPP


#include "cursor.hpp"
#include "div_ui_element.hpp"
#include "widget.hpp"


namespace editor {

class UIDialogBox : public Widget
{
    private:
        class Gizmo final
        {
            public:
                eCursor const       _cursor = eCursor::Arrow;
                size_t              _eventID = 0U;
                bool                _isHovered = false;
                Rect                _rect {};

            public:
                Gizmo () = delete;

                Gizmo ( Gizmo const & ) = delete;
                Gizmo &operator = ( Gizmo const & ) = delete;

                Gizmo ( Gizmo && ) = delete;
                Gizmo &operator = ( Gizmo && ) = delete;

                explicit Gizmo ( eCursor cursor ) noexcept;

                ~Gizmo () = default;

                [[nodiscard]] bool OnMouseMove ( MessageQueue &messageQueue, MouseMoveEvent const &event ) noexcept;
        };

    protected:
        DIVUIElement                _div;
        bool                        _dragState = false;

    private:
        std::vector<float>          _lineHeights { 1U, 0.0F };
        size_t                      _eventID = 0U;

        Rect                        _initialRect {};
        int32_t                     _initialX = 0;
        int32_t                     _initialY = 0;

        bool                        _isChanged = false;

        uint32_t                    _leftMask = 0U;
        uint32_t                    _topMask = 0U;
        uint32_t                    _rightMask = 0U;
        uint32_t                    _bottomMask = 0U;

        pbr::LengthValue            _minWidthCSS { pbr::LengthValue::eType::PX, 0.0F };
        pbr::LengthValue            _minHeightCSS { pbr::LengthValue::eType::PX, 0.0F };

        int32_t                     _minWidth = 0;
        int32_t                     _minHeight = 0;

        int32_t                     _safeDX = 0;
        int32_t                     _safeDY = 0;

        Gizmo                       _dragArea { eCursor::Cross };
        Gizmo                       _resizeUp { eCursor::NorthSouth };
        Gizmo                       _resizeDown { eCursor::NorthSouth };
        Gizmo                       _resizeLeft { eCursor::WestEast };
        Gizmo                       _resizeRight { eCursor::WestEast };
        Gizmo                       _resizeTopLeft { eCursor::NorthWestSouthEast };
        Gizmo                       _resizeTopRight { eCursor::NorthEastSouthWest };
        Gizmo                       _resizeBottomLeft { eCursor::NorthEastSouthWest };
        Gizmo                       _resizeBottomRight { eCursor::NorthWestSouthEast };

    public:
        UIDialogBox () = delete;

        UIDialogBox ( UIDialogBox const & ) = delete;
        UIDialogBox &operator = ( UIDialogBox const & ) = delete;

        UIDialogBox ( UIDialogBox && ) = delete;
        UIDialogBox &operator = ( UIDialogBox && ) = delete;

        void SetRect ( Rect const &rect ) noexcept;
        void SetMinSize ( pbr::LengthValue const &width, pbr::LengthValue const &height ) noexcept;

    protected:
        explicit UIDialogBox ( MessageQueue &messageQueue, std::string &&name ) noexcept;
        ~UIDialogBox () override = default;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;

    private:
        [[nodiscard]] LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
            pbr::FontStorage &fontStorage
        ) noexcept override;

        [[nodiscard]] bool UpdateCache ( pbr::FontStorage &fontStorage,
            VkExtent2D const &viewport
        ) noexcept override;

        void DoDrag ( MouseMoveEvent const &event ) noexcept;
        void DoHover ( MouseMoveEvent const &event ) noexcept;
        void UpdateAreas () noexcept;
        void UpdateMinSize () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_DIALOG_BOX_HPP
