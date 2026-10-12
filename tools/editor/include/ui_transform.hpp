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
        using CloseHandler = std::move_only_function<void ()>;

    private:
        DIVUIElement        _headerLine;
        UILabel             _headerText;
        UICloseButton       _closeButton;
        UIEditBox           _x;
        UIEditBox           _y;
        UIEditBox           _z;
        UISeparator         _separator;
        UIEditBox           _scaleX;
        UIEditBox           _scaleY;
        UIEditBox           _scaleZ;
        CloseHandler        _onClose;

    public:
        UITransform () = delete;

        UITransform ( UITransform const & ) = delete;
        UITransform &operator = ( UITransform const & ) = delete;

        UITransform ( UITransform && ) = delete;
        UITransform &operator = ( UITransform && ) = delete;

        explicit UITransform ( CloseHandler &&onClose ) noexcept;

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

        void CancelX ( std::string const &value ) noexcept;
        void ChangeX ( std::string const &value ) noexcept;
        void NewX ( std::string const &value ) noexcept;
        void NextX ( std::string const &value ) noexcept;

        void CancelY ( std::string const &value ) noexcept;
        void ChangeY ( std::string const &value ) noexcept;
        void NewY ( std::string const &value ) noexcept;
        void NextY ( std::string const &value ) noexcept;

        void CancelZ ( std::string const &value ) noexcept;
        void ChangeZ ( std::string const &value ) noexcept;
        void NewZ ( std::string const &value ) noexcept;
        void NextZ ( std::string const &value ) noexcept;

        void CancelScaleX ( std::string const &value ) noexcept;
        void ChangeScaleX ( std::string const &value ) noexcept;
        void NewScaleX ( std::string const &value ) noexcept;
        void NextScaleX ( std::string const &value ) noexcept;

        void CancelScaleY ( std::string const &value ) noexcept;
        void ChangeScaleY ( std::string const &value ) noexcept;
        void NewScaleY ( std::string const &value ) noexcept;
        void NextScaleY ( std::string const &value ) noexcept;

        void CancelScaleZ ( std::string const &value ) noexcept;
        void ChangeScaleZ ( std::string const &value ) noexcept;
        void NewScaleZ ( std::string const &value ) noexcept;
        void NextScaleZ ( std::string const &value ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_TRANSFORM_HPP
