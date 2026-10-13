#ifndef EDITOR_UI_TRANSFORM_HPP
#define EDITOR_UI_TRANSFORM_HPP


#include "ui_close_button.hpp"
#include "ui_dialog_box.hpp"
#include "ui_edit_box.hpp"
#include "ui_label.hpp"
#include "ui_separator.hpp"


namespace editor {

class UITransform final : public UIDialogBox
{
    public:
        using ActionHandler = std::move_only_function<void ()>;

    private:
        DIVUIElement                _headerLine;
        UILabel                     _headerText;
        UICloseButton               _closeButton;

        UIEditBox                   _x;
        UIEditBox                   _y;
        UIEditBox                   _z;

        UISeparator                 _separator;

        UIEditBox                   _scaleX;
        UIEditBox                   _scaleY;
        UIEditBox                   _scaleZ;

        ActionHandler               _onClose;

        UIEditBox::ValueHandler     _onNextX = nullptr;
        UIEditBox::ValueHandler     _onNextY = nullptr;
        UIEditBox::ValueHandler     _onNextZ = nullptr;

        UIEditBox::ValueHandler     _onNextScaleX = nullptr;
        UIEditBox::ValueHandler     _onNextScaleY = nullptr;
        UIEditBox::ValueHandler     _onNextScaleZ = nullptr;

    public:
        UITransform () = delete;

        UITransform ( UITransform const & ) = delete;
        UITransform &operator = ( UITransform const & ) = delete;

        UITransform ( UITransform && ) = delete;
        UITransform &operator = ( UITransform && ) = delete;

        explicit UITransform ( ActionHandler &&onClose,
            ActionHandler &&onCancelX,
            UIEditBox::ValueHandler &&onChangeX,
            UIEditBox::ValueHandler &&onNewX,
            UIEditBox::ValueHandler &&onNextX,

            ActionHandler &&onCancelY,
            UIEditBox::ValueHandler &&onChangeY,
            UIEditBox::ValueHandler &&onNewY,
            UIEditBox::ValueHandler &&onNextY,

            ActionHandler &&onCancelZ,
            UIEditBox::ValueHandler &&onChangeZ,
            UIEditBox::ValueHandler &&onNewZ,
            UIEditBox::ValueHandler &&onNextZ,

            ActionHandler &&onCancelScaleX,
            UIEditBox::ValueHandler &&onChangeScaleX,
            UIEditBox::ValueHandler &&onNewScaleX,
            UIEditBox::ValueHandler &&onNextScaleX,

            ActionHandler &&onCancelScaleY,
            UIEditBox::ValueHandler &&onChangeScaleY,
            UIEditBox::ValueHandler &&onNewScaleY,
            UIEditBox::ValueHandler &&onNextScaleY,

            ActionHandler &&onCancelScaleZ,
            UIEditBox::ValueHandler &&onChangeScaleZ,
            UIEditBox::ValueHandler &&onNewScaleZ,
            UIEditBox::ValueHandler &&onNextScaleZ
        ) noexcept;

        ~UITransform () override = default;

        void GetRect ( Rect &target ) const noexcept;
        void Close () noexcept;

        void SetX ( std::string &&value ) noexcept;
        void SetY ( std::string &&value ) noexcept;
        void SetZ ( std::string &&value ) noexcept;

        void SetScaleX ( std::string &&value ) noexcept;
        void SetScaleY ( std::string &&value ) noexcept;
        void SetScaleZ ( std::string &&value ) noexcept;

    private:
        [[nodiscard]] bool HasChild ( Widget const &child ) const noexcept override;
        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;

        void NextX ( std::string const &value ) noexcept;
        void NextY ( std::string const &value ) noexcept;
        void NextZ ( std::string const &value ) noexcept;

        void NextScaleX ( std::string const &value ) noexcept;
        void NextScaleY ( std::string const &value ) noexcept;
        void NextScaleZ ( std::string const &value ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_TRANSFORM_HPP
