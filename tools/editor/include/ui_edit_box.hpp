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

        enum class eLetterType : uint8_t
        {
            AlphaNumeric,
            Punctuation,
            Whitespace
        };

    private:
        std::u32string                      _content {};
        pbr::FontStorage::StringMetrics     _metrics {};
        pbr::FontStorage                    &_fontStorage;

        DIVUIElement                        _lineDIV;
        DIVUIElement                        _columnDIV;

        DIVUIElement                        _captionDIV;
        TextUIElement                       _captionText;

        DIVUIElement                        _valueDIV;

        DIVUIElement                        _cursorDIV;
        DIVUIElement                        _selectionDIV;

        DIVUIElement                        _textDIV;
        TextUIElement                       _text;

        // rule: index before symbol
        int32_t                             _cursor = 0;
        int32_t                             _selection = 0;

        size_t                              _eventID = 0U;
        MouseMoveHandler                    _onMouseMove = &UIEditBox::OnMouseMoveNormal;
        UpdateRectHandler                   _updateRect = &UIEditBox::UpdatedRectNormal;

        std::unique_ptr<Timer>              _blink {};

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
        void OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept override;
        void OnMouseLeave () noexcept override;
        void OnTyping ( char32_t character ) noexcept override;

        void OnMouseMoveEdit ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectEdit () noexcept;

        void OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectNormal () noexcept;

        void Append ( char32_t character ) noexcept;
        void Copy () noexcept;
        void Cut () noexcept;
        void Erase ( int32_t offset ) noexcept;
        void Paste () noexcept;

        [[nodiscard]] std::pair<int32_t, int32_t> GetSelection () const noexcept;
        void ModifySelection ( int32_t offset, int32_t cursorLimit ) noexcept;
        void MoveCursor ( int32_t cursor, bool cancelSelection ) noexcept;
        void OffsetCursor ( int32_t offset, KeyModifier modifier ) noexcept;

        void JumpOverWord ( int32_t offset, bool cancelSelection ) noexcept;
        [[nodiscard]] int32_t JumpOverWordLeft ( int32_t limit ) const noexcept;
        [[nodiscard]] int32_t JumpOverWordRight ( int32_t limit ) const noexcept;

        // The method returns true if selected content presents. Otherwise the method returns false.
        [[nodiscard]] bool RemoveSelectedContent () noexcept;

        void ResetBlinkTimer () noexcept;
        void SelectAll () noexcept;

        void SwitchToEditState () noexcept;
        void SwitchToNormalState () noexcept;

        void UpdateCursor () noexcept;
        void UpdateMetrics () noexcept;

        [[nodiscard]] static eLetterType ResolveLetterType ( char32_t c ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_EDIT_BOX_HPP
