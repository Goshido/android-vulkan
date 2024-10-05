#ifndef EDITOR_UI_DIALOG_BOX_HPP
#define EDITOR_UI_DIALOG_BOX_HPP


#include "widget.hpp"


namespace editor {

class UIDialogBox final : public Widget
{
    private:
        Rect    _dragArea {};
        Rect    _resizeUp {};
        Rect    _resizeDown {};
        Rect    _resizeLeft {};
        Rect    _resizeRight {};

        Rect    _resizeTopLeft {};
        Rect    _resizeTopRight {};
        Rect    _resizeBottomLeft {};
        Rect    _resizeBottomRight {};

        bool    _isChanged = false;

    public:
        explicit UIDialogBox () = default;

        UIDialogBox ( UIDialogBox const & ) = delete;
        UIDialogBox &operator = ( UIDialogBox const & ) = delete;

        UIDialogBox ( UIDialogBox && ) = delete;
        UIDialogBox &operator = ( UIDialogBox && ) = delete;

        ~UIDialogBox () override = default;

        void ApplyLayout () noexcept;
        void SetRect ( Rect const& rect ) noexcept;

    private:
        void OnButtonDown () noexcept override;
        void OnButtonUp () noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_DIALOG_BOX_HPP
