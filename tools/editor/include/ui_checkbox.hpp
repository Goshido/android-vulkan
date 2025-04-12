#ifndef EDITOR_UI_CHECKBOX_HPP
#define EDITOR_UI_CHECKBOX_HPP


#include "text_ui_element.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager;

class UICheckbox final : public Widget
{
    public:
        enum class eState : uint8_t
        {
            Check,
            Unckeck,
            Multi
        };

        using Callback = std::function<void ( eState state )>;

    private:
        DIVUIElement        _lineDIV;
        DIVUIElement        _columnDIV;

        DIVUIElement        _captionDIV;
        TextUIElement       _captionText;

        DIVUIElement        _valueDIV;
        TextUIElement       _valueIcon;

        Callback            _callback {};
        size_t              _eventID = 0U;

        eState              _state = eState::Check;

    public:
        UICheckbox () = delete;

        UICheckbox ( UICheckbox const & ) = delete;
        UICheckbox &operator = ( UICheckbox const & ) = delete;

        UICheckbox ( UICheckbox && ) = delete;
        UICheckbox &operator = ( UICheckbox && ) = delete;

        explicit UICheckbox ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            std::string_view caption,
            std::string &&name
        ) noexcept;

        ~UICheckbox () override = default;

        void OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;

    private:
        void OnMouseLeave () noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_CHECKBOX_HPP
