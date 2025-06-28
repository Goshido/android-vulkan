#ifndef EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
#define EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP


#include <pbr/div_ui_element.hpp>


namespace editor {

class AppendUIChildElementEvent final
{
    private:
        pbr::UIElement          &_element;
        pbr::DIVUIElement       &_parent;

    public:
        AppendUIChildElementEvent () = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent const & ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent const & ) = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent && ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent && ) = delete;

        explicit AppendUIChildElementEvent ( pbr::DIVUIElement &parent, pbr::UIElement &element ) noexcept;

        ~AppendUIChildElementEvent () = default;

        void Action () noexcept;
};

} // namespace editor


#endif // EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
