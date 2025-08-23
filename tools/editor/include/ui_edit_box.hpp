#ifndef EDITOR_UI_EDIT_BOX_HPP
#define EDITOR_UI_EDIT_BOX_HPP


#include "text_ui_element.hpp"
#include "timer.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIEditBox final : public Widget
{
    public:
        using Callback = std::function<void ( std::string const &value )>;

    private:
        using MouseButtonHandler = void ( UIEditBox::* ) ( MouseButtonEvent const &event ) noexcept;
        using MouseLeaveHandler  = void ( UIEditBox::* ) () noexcept;
        using MouseMoveHandler = void ( UIEditBox::* ) ( MouseMoveEvent const &event ) noexcept;
        using UpdateRectHandler = void ( UIEditBox::* ) () noexcept;

        enum class eLetterType : uint8_t
        {
            AlphaNumeric,
            Punctuation,
            Whitespace
        };

    private:
        std::string                                 _committed {};
        std::u32string                              _content {};

        // FUCK - remove namespace
        pbr::android::FontStorage::StringMetrics    _metrics {};
        pbr::android::FontStorage                   &_fontStorage;

        DIVUIElement                                _lineDIV;
        DIVUIElement                                _columnDIV;

        DIVUIElement                                _captionDIV;
        TextUIElement                               _captionText;

        DIVUIElement                                _valueDIV;

        DIVUIElement                                _cursorDIV;
        DIVUIElement                                _selectionDIV;

        DIVUIElement                                _textDIV;
        TextUIElement                               _text;

        // rule: index before symbol
        int32_t                                     _cursor = 0;
        int32_t                                     _selection = 0;
        bool                                        _leftMouseButtonPressed = false;

        size_t                                      _eventID = 0U;
        MouseButtonHandler                          _onDoubleClick = &UIEditBox::OnDoubleClickNormal;
        MouseButtonHandler                          _onMouseKeyDown = &UIEditBox::OnMouseButtonDownNormal;
        MouseButtonHandler                          _onMouseKeyUp = &UIEditBox::OnMouseButtonUpNormal;
        MouseLeaveHandler                           _onMouseLeave = &UIEditBox::OnMouseLeaveNormal;
        MouseMoveHandler                            _onMouseMove = &UIEditBox::OnMouseMoveNormal;
        UpdateRectHandler                           _updateRect = &UIEditBox::UpdatedRectNormal;

        std::unique_ptr<Timer>                      _blink {};
        Callback                                    _callback {};

    public:
        UIEditBox () = delete;

        UIEditBox ( UIEditBox const & ) = delete;
        UIEditBox &operator = ( UIEditBox const & ) = delete;

        UIEditBox ( UIEditBox && ) = delete;
        UIEditBox &operator = ( UIEditBox && ) = delete;

        explicit UIEditBox ( MessageQueue &messageQueue,
            DIVUIElement &parent,

            // FUCK - remove namespace
            pbr::android::FontStorage &fontStorage,

            std::string_view caption,
            std::string_view value,
            std::string &&name
        ) noexcept;

        ~UIEditBox () = default;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;

    private:
        void ApplyClipboard ( std::u32string const &text ) noexcept override;
        void OnDoubleClick ( MouseButtonEvent const &event ) noexcept override;
        void OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept override;
        void OnMouseLeave () noexcept override;
        void OnTyping ( char32_t character ) noexcept override;

        void OnDoubleClickEdit ( MouseButtonEvent const &event ) noexcept;
        void OnMouseButtonDownEdit ( MouseButtonEvent const &event ) noexcept;
        void OnMouseButtonUpEdit ( MouseButtonEvent const &event ) noexcept;
        void OnMouseLeaveEdit () noexcept;
        void OnMouseMoveEdit ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectEdit () noexcept;

        void OnDoubleClickNormal ( MouseButtonEvent const &event ) noexcept;
        void OnMouseButtonDownNormal ( MouseButtonEvent const &event ) noexcept;
        void OnMouseButtonUpNormal ( MouseButtonEvent const &event ) noexcept;
        void OnMouseLeaveNormal () noexcept;
        void OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectNormal () noexcept;

        void Append ( char32_t character ) noexcept;
        void Commit () noexcept;
        void Copy () noexcept;
        void Cut () noexcept;
        void Erase ( int32_t offset ) noexcept;
        void Paste () noexcept;

        [[nodiscard]] int32_t FindClosestSymbol ( int32_t x ) const noexcept;
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
