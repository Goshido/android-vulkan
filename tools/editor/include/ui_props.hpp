#ifndef EDITOR_UI_PROPS_HPP
#define EDITOR_UI_PROPS_HPP


#include "ui_checkbox.hpp"
#include "ui_close_button.hpp"
#include "ui_dialog_box.hpp"
#include "ui_label.hpp"


namespace editor {

class UIProps final : public UIDialogBox
{
    private:
        pbr::DIVUIElement       _headerLine;
        UILabel                 _headerText;
        UICloseButton           _closeButton;
        UICheckbox              _checkbox;

    public:
        explicit UIProps ( MessageQueue &messageQueue ) noexcept;

        UIProps ( UIProps const & ) = delete;
        UIProps &operator = ( UIProps const & ) = delete;

        UIProps ( UIProps && ) = delete;
        UIProps &operator = ( UIProps && ) = delete;

        ~UIProps () override = default;

    private:
        void OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;

        void OnCheckBox ( UICheckbox::eState state ) noexcept;
        void OnClose () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_PROPS_HPP
