#ifndef EDITOR_UI_PROPERTY_DIALOG_HPP
#define EDITOR_UI_PROPERTY_DIALOG_HPP


#include "ui_close_button.hpp"
#include "ui_dialog_box.hpp"
#include "ui_edit_box.hpp"
#include "ui_label.hpp"
#include "ui_separator.hpp"


namespace editor {

class UIPropertyDialog final : public UIDialogBox
{
    public:
        using ActionHandler = std::move_only_function<void ()>;

    private:
        DIVUIElement                                    _headerLine;
        UILabel                                         _headerText;
        UICloseButton                                   _closeButton;
        ActionHandler                                   _onClose = nullptr;

        std::vector<std::unique_ptr<UIEditBox>>         _editBoxes {};
        std::vector<std::unique_ptr<DIVUIElement>>      _divs {};
        std::vector<std::unique_ptr<UILabel>>           _labels {};
        std::vector<std::unique_ptr<UISeparator>>       _separators {};

    public:
        UIPropertyDialog () = delete;

        UIPropertyDialog ( UIPropertyDialog const & ) = delete;
        UIPropertyDialog &operator = ( UIPropertyDialog const & ) = delete;

        UIPropertyDialog ( UIPropertyDialog && ) = delete;
        UIPropertyDialog &operator = ( UIPropertyDialog && ) = delete;

        explicit UIPropertyDialog ( ActionHandler &&onClose ) noexcept;

        ~UIPropertyDialog () override = default;

        void GetRect ( Rect &target ) const noexcept;
        void Close () noexcept;
        [[nodiscard]] DIVUIElement &GetBody () noexcept;

        void Append ( std::unique_ptr<UIEditBox> control ) noexcept;
        void AddSeparator () noexcept;
        void AddComponentHeader ( std::string const &name, std::string_view glyph ) noexcept;

    private:
        [[nodiscard]] bool HasChild ( Widget const &child ) const noexcept override;
        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_PROPERTY_DIALOG_HPP
