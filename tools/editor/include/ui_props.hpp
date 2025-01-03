#ifndef EDITOR_UI_PROPS_HPP
#define EDITOR_UI_PROPS_HPP


#include "ui_dialog_box.hpp"
#include "ui_label.hpp"


namespace editor {

class UIProps final : public UIDialogBox
{
    private:
        std::unique_ptr<UILabel>    _header {};

    public:
        explicit UIProps ( MessageQueue &messageQueue ) noexcept;

        UIProps ( UIProps const & ) = delete;
        UIProps &operator = ( UIProps const & ) = delete;

        UIProps ( UIProps && ) = delete;
        UIProps &operator = ( UIProps && ) = delete;

        ~UIProps () override = default;
};

} // namespace editor


#endif // EDITOR_UI_PROPS_HPP
