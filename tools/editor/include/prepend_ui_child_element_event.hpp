#ifndef EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP
#define EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP


#include <platform/windows/pbr/div_ui_element.hpp>


namespace editor {

class PrependUIChildElementEvent final
{
    private:
        // FUCK - remove namespace
        pbr::windows::UIElement         &_element;

        // FUCK - remove namespace
        pbr::windows::DIVUIElement      &_parent;

    public:
        PrependUIChildElementEvent () = delete;

        PrependUIChildElementEvent ( PrependUIChildElementEvent const & ) = delete;
        PrependUIChildElementEvent &operator = ( PrependUIChildElementEvent const & ) = delete;

        PrependUIChildElementEvent ( PrependUIChildElementEvent && ) = delete;
        PrependUIChildElementEvent &operator = ( PrependUIChildElementEvent && ) = delete;

        // FUCK - remove namespace
        explicit PrependUIChildElementEvent ( pbr::windows::DIVUIElement &parent,
            pbr::windows::UIElement &element
        ) noexcept;

        ~PrependUIChildElementEvent () = default;

        void Action () noexcept;
};

} // namespace editor


#endif // EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP
