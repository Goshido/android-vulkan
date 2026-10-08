#ifndef EDITOR_UI_TRANSFORM_HPP
#define EDITOR_UI_TRANSFORM_HPP


#include "save_state.hpp"
#include "ui_close_button.hpp"
#include "ui_dialog_box.hpp"
#include "ui_edit_box.hpp"
#include "ui_label.hpp"
#include "ui_separator.hpp"


namespace editor {

class UITransform final : public UIDialogBox
{
    private:
        DIVUIElement        _headerLine;
        UILabel             _headerText;
        UICloseButton       _closeButton;
        UIEditBox           _locationX;
        UIEditBox           _locationY;
        UIEditBox           _locationZ;
        UISeparator         _separator;
        UIEditBox           _scaleX;
        UIEditBox           _scaleY;
        UIEditBox           _scaleZ;

    public:
        UITransform () = delete;

        UITransform ( UITransform const & ) = delete;
        UITransform &operator = ( UITransform const & ) = delete;

        UITransform ( UITransform && ) = delete;
        UITransform &operator = ( UITransform && ) = delete;

        explicit UITransform ( SaveState::Container const &save ) noexcept;

        ~UITransform () override = default;

        void Save ( SaveState::Container &save ) const noexcept;

    private:
        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;

        void OnClose () noexcept;

        void OnLocationX ( std::string const &value ) noexcept;
        void OnLocationY ( std::string const &value ) noexcept;
        void OnLocationZ ( std::string const &value ) noexcept;

        void OnScaleX ( std::string const &value ) noexcept;
        void OnScaleY ( std::string const &value ) noexcept;
        void OnScaleZ ( std::string const &value ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_TRANSFORM_HPP
