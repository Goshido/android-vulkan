#ifndef EDITOR_UI_EDIT_BOX_HPP
#define EDITOR_UI_EDIT_BOX_HPP


#include "widget.hpp"
#include "text_ui_element.hpp"


namespace editor {

class UIEditBox final : public Widget
{
    private:
        using MouseMoveHandler = void ( UIEditBox::* ) ( MouseMoveEvent const &event ) noexcept;
        using UpdateRectHandler = void ( UIEditBox::* ) () noexcept;

    private:
        DIVUIElement            _lineDIV;
        DIVUIElement            _columnDIV;

        DIVUIElement            _captionDIV;
        TextUIElement           _captionText;

        DIVUIElement            _valueDIV;

        DIVUIElement            _textDIV;
        TextUIElement           _text;

        size_t                  _eventID = 0U;
        MouseMoveHandler        _onMouseMove = &UIEditBox::OnMouseMoveNormal;
        UpdateRectHandler       _updateRect = &UIEditBox::UpdatedRectNormal;

    public:
        UIEditBox () = delete;

        UIEditBox ( UIEditBox const & ) = delete;
        UIEditBox &operator = ( UIEditBox const & ) = delete;

        UIEditBox ( UIEditBox && ) = delete;
        UIEditBox &operator = ( UIEditBox && ) = delete;

        explicit UIEditBox ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            std::string_view caption,
            std::string_view value,
            std::string &&name
        ) noexcept;

        ~UIEditBox () = default;

        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

    private:
        void OnMouseLeave () noexcept override;

        void OnMouseMoveEdit ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectEdit () noexcept;

        void OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectNormal () noexcept;

        void SwitchToEditState () noexcept;
        void SwitchToNormalState () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_EDIT_BOX_HPP
