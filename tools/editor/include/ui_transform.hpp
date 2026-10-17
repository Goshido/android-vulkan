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

        ActionHandler               _onClose = nullptr;
        ActionHandler               _onRevertLocation = nullptr;
        ActionHandler               _onRevertScale = nullptr;

        UIEditBox::ValueHandler     _onNewX = nullptr;
        UIEditBox::ValueHandler     _onNewY = nullptr;
        UIEditBox::ValueHandler     _onNewZ = nullptr;

        UIEditBox::ValueHandler     _onNewScaleX = nullptr;
        UIEditBox::ValueHandler     _onNewScaleY = nullptr;
        UIEditBox::ValueHandler     _onNewScaleZ = nullptr;

    public:
        UITransform () = delete;

        UITransform ( UITransform const & ) = delete;
        UITransform &operator = ( UITransform const & ) = delete;

        UITransform ( UITransform && ) = delete;
        UITransform &operator = ( UITransform && ) = delete;

        explicit UITransform ( ActionHandler &&onClose,
            ActionHandler &&onRevertLocation,
            ActionHandler &&onRevertScale,

            UIEditBox::ValueHandler &&onChangeX,
            UIEditBox::ValueHandler &&onNewX,

            UIEditBox::ValueHandler &&onChangeY,
            UIEditBox::ValueHandler &&onNewY,

            UIEditBox::ValueHandler &&onChangeZ,
            UIEditBox::ValueHandler &&onNewZ,

            UIEditBox::ValueHandler &&onChangeScaleX,
            UIEditBox::ValueHandler &&onNewScaleX,

            UIEditBox::ValueHandler &&onChangeScaleY,
            UIEditBox::ValueHandler &&onNewScaleY,

            UIEditBox::ValueHandler &&onChangeScaleZ,
            UIEditBox::ValueHandler &&onNewScaleZ
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

        static void Next ( std::string const &value, UIEditBox &axis, UIEditBox::ValueHandler &handler ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_TRANSFORM_HPP
