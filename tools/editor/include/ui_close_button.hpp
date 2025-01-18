#ifndef EDITOR_UI_CLOSE_BUTTON_HPP
#define EDITOR_UI_CLOSE_BUTTON_HPP


#include <pbr/div_ui_element.hpp>
#include <pbr/text_ui_element.hpp>
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager;

class UICloseButton final : public Widget
{
    public:
        using Callback = std::function<void ()>;

    private:
        pbr::DIVUIElement       _base;

        pbr::DIVUIElement       _backgroundDIV;
        pbr::TextUIElement      _backgroundText;

        pbr::DIVUIElement       _borderDIV;
        pbr::TextUIElement      _borderText;

        pbr::DIVUIElement       _crossDIV;
        pbr::TextUIElement      _crossText;

        Callback                _callback {};
        size_t                  _eventID = 0U;

    public:
        UICloseButton () = delete;

        UICloseButton ( UICloseButton const & ) = delete;
        UICloseButton &operator = ( UICloseButton const & ) = delete;

        UICloseButton ( UICloseButton && ) = delete;
        UICloseButton &operator = ( UICloseButton && ) = delete;

        explicit UICloseButton ( MessageQueue &messageQueue, pbr::DIVUIElement &parent, std::string &&name ) noexcept;

        ~UICloseButton () override = default;

        void OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;
        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;

    private:
        void OnMouseLeave () noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_CLOSE_BUTTON_HPP
