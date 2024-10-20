#ifndef EDITOR_UI_DIALOG_BOX_HPP
#define EDITOR_UI_DIALOG_BOX_HPP


#include "widget.hpp"
#include <pbr/div_ui_element.hpp>


namespace editor {

class UIDialogBox final : public Widget
{
    private:
        Rect                    _dragArea {};
        Rect                    _resizeUp {};
        Rect                    _resizeDown {};
        Rect                    _resizeLeft {};
        Rect                    _resizeRight {};

        Rect                    _resizeTopLeft {};
        Rect                    _resizeTopRight {};
        Rect                    _resizeBottomLeft {};
        Rect                    _resizeBottomRight {};

        bool                    _isChanged = false;

        std::vector<float>      _lineHeights { 1U, 0.0F };
        pbr::DIVUIElement       _div { nullptr, {} };

    public:
        explicit UIDialogBox () noexcept;

        UIDialogBox ( UIDialogBox const & ) = delete;
        UIDialogBox &operator = ( UIDialogBox const & ) = delete;

        UIDialogBox ( UIDialogBox && ) = delete;
        UIDialogBox &operator = ( UIDialogBox && ) = delete;

        ~UIDialogBox () override = default;

        void SetRect ( Rect const& rect ) noexcept;

    private:
        void OnButtonDown () noexcept override;
        void OnButtonUp () noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

        [[nodiscard]] LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
            pbr::FontStorage &fontStorage
        ) noexcept override;

        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;

        [[nodiscard]] bool UpdateCache ( pbr::FontStorage &fontStorage,
            VkExtent2D const &viewport
        ) noexcept override;

        void UpdateAreas () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_DIALOG_BOX_HPP
