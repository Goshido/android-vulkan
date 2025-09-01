#ifndef EDITOR_UI_PROPS_HPP
#define EDITOR_UI_PROPS_HPP


#include "ui_check_box.hpp"
#include "ui_close_button.hpp"
#include "ui_combo_box.hpp"
#include "ui_dialog_box.hpp"
#include "ui_edit_box.hpp"
#include "ui_label.hpp"
#include "ui_slider.hpp"


namespace editor {

class UIProps final : public UIDialogBox
{
    private:
        // FUCK - remove namespace
        pbr::android::FontStorage       &_fontStorage;

        DIVUIElement                    _headerLine;
        UILabel                         _headerText;
        UICloseButton                   _closeButton;
        UICheckBox                      _checkBox;
        UIComboBox                      _comboBox;
        UISlider                        _slider;
        UIEditBox                       _editBox;

    public:
        // FUCK - remove namespace
        explicit UIProps ( MessageQueue &messageQueue, pbr::android::FontStorage &fontStorage ) noexcept;

        UIProps ( UIProps const & ) = delete;
        UIProps &operator = ( UIProps const & ) = delete;

        UIProps ( UIProps && ) = delete;
        UIProps &operator = ( UIProps && ) = delete;

        ~UIProps () override = default;

    private:
        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

        // FUCK - remove namespace
        void Submit ( pbr::android::UIElement::SubmitInfo &info ) noexcept override;

        void OnCheckBox ( UICheckBox::eState state ) noexcept;
        void OnComboBox ( UIComboBox::ID id ) noexcept;
        void OnClose () noexcept;
        void OnEditBox ( std::string const &value ) noexcept;
        void OnSlider ( double value ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_PROPS_HPP
