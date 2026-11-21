#ifndef EDITOR_UI_PROPERTY_DIALOG_HPP
#define EDITOR_UI_PROPERTY_DIALOG_HPP


#include "string_property.hpp"
#include "ui_close_button.hpp"
#include "ui_component_header.hpp"
#include "ui_dialog_box.hpp"
#include "ui_edit_box.hpp"
#include "ui_separator.hpp"


namespace editor {

class UIPropertyDialog final : public UIDialogBox
{
    public:
        using ActionHandler = std::move_only_function<void ()>;

    private:
        class EditBoxInfo final
        {
            public:
                std::unique_ptr<UIEditBox>                  _editBox {};

            private:
                StringProperty*                             _property = nullptr;
                Property::Connection                        _connection = 42U;

            public:
                EditBoxInfo () = delete;

                EditBoxInfo ( EditBoxInfo const & ) = delete;
                EditBoxInfo &operator = ( EditBoxInfo const & ) = delete;

                EditBoxInfo ( EditBoxInfo &&other ) noexcept;
                EditBoxInfo &operator = ( EditBoxInfo &&other ) noexcept;

                explicit EditBoxInfo ( DIVUIElement &root,
                    std::string_view caption,
                    StringProperty &property
                ) noexcept;

                ~EditBoxInfo () noexcept;

                void Disconnect () noexcept;
                void DumpLayout ( pbr::SVGWriter &writer ) const noexcept;
        };

    private:
        DIVUIElement                                        _headerLine;
        UILabel                                             _headerText;
        UICloseButton                                       _closeButton;
        ActionHandler                                       _onClose = nullptr;

        std::vector<EditBoxInfo>                            _editBoxes {};
        std::vector<std::unique_ptr<UIComponentHeader>>     _headers {};
        std::vector<std::unique_ptr<UILabel>>               _labels {};
        std::vector<std::unique_ptr<UISeparator>>           _separators {};

        bool                                                _firstSubmit = true;

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
        void Disconnect () noexcept;
        [[nodiscard]] DIVUIElement &GetBody () noexcept;

        void Append ( std::string_view name, StringProperty &property ) noexcept;

        void Append ( StringProperty &componentName,
            bool &componentCollapse,
            bool &componentSelect,
            std::string_view componentGlyph
        ) noexcept;

        void AppendSeparator () noexcept;

    private:
        [[nodiscard]] bool HasChild ( Widget const &child ) const noexcept override;
        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

        void DumpLayout ( pbr::SVGWriter &writer ) const noexcept override;
        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_PROPERTY_DIALOG_HPP
