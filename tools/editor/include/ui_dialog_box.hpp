#ifndef EDITOR_UI_DIALOG_BOX_HPP
#define EDITOR_UI_DIALOG_BOX_HPP


#include "rect.hpp"
#include "widget.hpp"


namespace editor {

class UIDialogBox final : public Widget
{
    private:
        Rect    _body {};

        Rect    _dragArea {};
        Rect    _resizeUp {};
        Rect    _resizeDown {};
        Rect    _resizeLeft {};
        Rect    _resizeRight {};

        Rect    _resizeTopLeft {};
        Rect    _resizeTopRight {};
        Rect    _resizeBottomLeft {};
        Rect    _resizeBottomRight {};

    public:
        explicit UIDialogBox () = default;

        UIDialogBox ( UIDialogBox const & ) = default;
        UIDialogBox &operator = ( UIDialogBox const & ) = default;

        UIDialogBox ( UIDialogBox && ) = default;
        UIDialogBox &operator = ( UIDialogBox && ) = default;

        ~UIDialogBox () override = default;

    private:
        void OnButtonDown () noexcept override;
        void OnButtonUp () noexcept override;
        void OnMouseMove () noexcept override;

        void ApplyLayout () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_DIALOG_BOX_HPP
