#ifndef EDITOR_UI_COLLAPSE_BUTTON_HPP
#define EDITOR_UI_COLLAPSE_BUTTON_HPP


#include "text_ui_element.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UICollapseButton final : public Widget
{
    public:
        using Callback = std::move_only_function<void ( bool shrink )>;

    private:
        DIVUIElement        _div;
        TextUIElement       _icon;

        Callback            _callback {};
        size_t              _eventID = 0U;

        bool                _collapse = false;

    public:
        UICollapseButton () = delete;

        UICollapseButton ( UICollapseButton const & ) = delete;
        UICollapseButton &operator = ( UICollapseButton const & ) = delete;

        UICollapseButton ( UICollapseButton && ) = delete;
        UICollapseButton &operator = ( UICollapseButton && ) = delete;

        explicit UICollapseButton ( DIVUIElement &parent, bool collapse, std::string &&name ) noexcept;

        ~UICollapseButton () override = default;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;
        void DumpLayout ( pbr::SVGWriter &writer ) const noexcept override;

    private:
        void OnMouseLeave () noexcept override;
};

} // namespace editor


#endif // EDITOR_UI_COLLAPSE_BUTTON_HPP
