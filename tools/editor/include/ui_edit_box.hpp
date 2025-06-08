#ifndef EDITOR_UI_EDIT_BOX_HPP
#define EDITOR_UI_EDIT_BOX_HPP


#include "widget.hpp"
#include "text_ui_element.hpp"
#include <pbr/font_storage.hpp>
#include "timer.hpp"


namespace editor {

class UIEditBox final : public Widget
{
    private:
        using MouseMoveHandler = void ( UIEditBox::* ) ( MouseMoveEvent const &event ) noexcept;
        using UpdateRectHandler = void ( UIEditBox::* ) () noexcept;

    private:
        std::u32string              _content {};
        pbr::FontStorage            &_fontStorage;

        DIVUIElement                _lineDIV;
        DIVUIElement                _columnDIV;

        DIVUIElement                _captionDIV;
        TextUIElement               _captionText;

        DIVUIElement                _valueDIV;

        DIVUIElement                _cursorDIV;
        DIVUIElement                _selectionDIV;

        DIVUIElement                _textDIV;
        TextUIElement               _text;

        // rule: index before symbol
        int32_t                     _cursor = 0;
        int32_t                     _selection = 0;

        size_t                      _eventID = 0U;
        MouseMoveHandler            _onMouseMove = &UIEditBox::OnMouseMoveNormal;
        UpdateRectHandler           _updateRect = &UIEditBox::UpdatedRectNormal;

        std::unique_ptr<Timer>      _blink {};

    public:
        UIEditBox () = delete;

        UIEditBox ( UIEditBox const & ) = delete;
        UIEditBox &operator = ( UIEditBox const & ) = delete;

        UIEditBox ( UIEditBox && ) = delete;
        UIEditBox &operator = ( UIEditBox && ) = delete;

        explicit UIEditBox ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            pbr::FontStorage &fontStorage,
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

        void UpdateCursor () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_EDIT_BOX_HPP
