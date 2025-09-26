#ifndef EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
#define EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP


#include <platform/windows/pbr/div_ui_element.hpp>


namespace editor {

class AppendUIChildElementEvent final
{
    private:
        // FUCK - remove namespace
        pbr::windows::UIElement         &_element;

        // FUCK - remove namespace
        pbr::windows::DIVUIElement      &_parent;

    public:
        AppendUIChildElementEvent () = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent const & ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent const & ) = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent && ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent && ) = delete;

        // FUCK - remove namespace
        explicit AppendUIChildElementEvent ( pbr::windows::DIVUIElement &parent,
            pbr::windows::UIElement &element
        ) noexcept;

        ~AppendUIChildElementEvent () = default;

        void Action () noexcept;
};

} // namespace editor


#endif // EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
