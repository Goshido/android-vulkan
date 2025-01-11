#ifndef EDITOR_UI_PROPS_HPP
#define EDITOR_UI_PROPS_HPP


#include "ui_close_button.hpp"
#include "ui_dialog_box.hpp"
#include "ui_label.hpp"


namespace editor {

class UIProps final : public UIDialogBox
{
    private:
        std::unique_ptr<pbr::DIVUIElement>      _headerLine {};
        std::unique_ptr<UILabel>                _headerText {};
        std::unique_ptr<UICloseButton>          _closeButton {};

    public:
        explicit UIProps ( MessageQueue &messageQueue ) noexcept;

        UIProps ( UIProps const & ) = delete;
        UIProps &operator = ( UIProps const & ) = delete;

        UIProps ( UIProps && ) = delete;
        UIProps &operator = ( UIProps && ) = delete;

        ~UIProps () override = default;

    private:
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_PROPS_HPP
