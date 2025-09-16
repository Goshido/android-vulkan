#ifndef EDITOR_UI_CLOSE_BUTTON_HPP
#define EDITOR_UI_CLOSE_BUTTON_HPP


#include "text_ui_element.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UICloseButton final : public Widget
{
    public:
        using Callback = std::function<void ()>;

    private:
        DIVUIElement        _base;

        DIVUIElement        _backgroundDIV;
        TextUIElement       _backgroundText;

        DIVUIElement        _borderDIV;
        TextUIElement       _borderText;

        DIVUIElement        _crossDIV;
        TextUIElement       _crossText;

        Callback            _callback {};
        size_t              _eventID = 0U;

    public:
        UICloseButton () = delete;

        UICloseButton ( UICloseButton const & ) = delete;
        UICloseButton &operator = ( UICloseButton const & ) = delete;

        UICloseButton ( UICloseButton && ) = delete;
        UICloseButton &operator = ( UICloseButton && ) = delete;

        explicit UICloseButton ( MessageQueue &messageQueue, DIVUIElement &parent, std::string &&name ) noexcept;

        ~UICloseButton () override = default;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;

        // FUCK - remove it
        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;

        // FUCK - rename
        [[nodiscard]] pbr::CSSComputedValues &GetCSSEXT () noexcept;

    private:
        void OnMouseLeave () noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_CLOSE_BUTTON_HPP
