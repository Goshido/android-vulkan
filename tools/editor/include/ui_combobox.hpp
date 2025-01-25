#ifndef EDITOR_UI_COMBOBOX_HPP
#define EDITOR_UI_COMBOBOX_HPP


#include <pbr/div_ui_element.hpp>
#include <pbr/text_ui_element.hpp>
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager;

class UICombobox final : public Widget
{
    public:
        using Callback = std::function<void ( uint32_t )>;

    private:
        pbr::DIVUIElement       _lineDIV;
        pbr::DIVUIElement       _columnDIV;

        pbr::DIVUIElement       _captionDIV;
        pbr::TextUIElement      _captionText;

        pbr::DIVUIElement       _valueDIV;

        pbr::DIVUIElement       _textDIV;
        pbr::TextUIElement      _text;

        pbr::DIVUIElement       _iconDIV;
        pbr::TextUIElement      _icon;

        Callback                _callback {};
        size_t                  _eventID = 0U;

    public:
        UICombobox () = delete;

        UICombobox ( UICombobox const & ) = delete;
        UICombobox &operator = ( UICombobox const & ) = delete;

        UICombobox ( UICombobox && ) = delete;
        UICombobox &operator = ( UICombobox && ) = delete;

        explicit UICombobox ( MessageQueue &messageQueue,
            pbr::DIVUIElement &parent,
            std::string_view caption,
            std::string &&name
        ) noexcept;

        ~UICombobox () override = default;

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


#endif // EDITOR_UI_COMBOBOX_HPP
