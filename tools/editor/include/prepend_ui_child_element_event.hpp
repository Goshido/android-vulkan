#ifndef EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP
#define EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP


#include <pbr/div_ui_element.hpp>


namespace editor {

class PrependUIChildElementEvent final
{
    private:
        pbr::UIElement          &_element;
        pbr::DIVUIElement       &_parent;

    public:
        PrependUIChildElementEvent () = delete;

        PrependUIChildElementEvent ( PrependUIChildElementEvent const & ) = delete;
        PrependUIChildElementEvent &operator = ( PrependUIChildElementEvent const & ) = delete;

        PrependUIChildElementEvent ( PrependUIChildElementEvent && ) = delete;
        PrependUIChildElementEvent &operator = ( PrependUIChildElementEvent && ) = delete;

        explicit PrependUIChildElementEvent ( pbr::DIVUIElement &parent, pbr::UIElement &element  ) noexcept;

        ~PrependUIChildElementEvent () = default;

        void Action () noexcept;
};

} // namespace editor


#endif // EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP
