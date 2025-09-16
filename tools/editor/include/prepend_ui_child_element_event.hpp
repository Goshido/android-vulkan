#ifndef EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP
#define EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP


// FUCK - remove namespace
#include <platform/android/pbr/div_ui_element.hpp>
#include <platform/windows/pbr/div_ui_element.hpp>


namespace editor {

class PrependUIChildElementEvent final
{
    private:
        // FUCK remove it
        pbr::android::UIElement         &_element;

        // FUCK - remove namespace
        pbr::windows::UIElement         &_elementEXT;

        // FUCK remove it
        pbr::android::DIVUIElement      &_parent;

        // FUCK - remove namespace
        pbr::windows::DIVUIElement      &_parentEXT;

        // FUCK remove it
        bool                            _isEXT = false;

    public:
        PrependUIChildElementEvent () = delete;

        PrependUIChildElementEvent ( PrependUIChildElementEvent const & ) = delete;
        PrependUIChildElementEvent &operator = ( PrependUIChildElementEvent const & ) = delete;

        PrependUIChildElementEvent ( PrependUIChildElementEvent && ) = delete;
        PrependUIChildElementEvent &operator = ( PrependUIChildElementEvent && ) = delete;

        // FUCK - remove it
        explicit PrependUIChildElementEvent ( pbr::android::DIVUIElement &parent,
            pbr::android::UIElement &element
        ) noexcept;

        // FUCK - remove namespace
        explicit PrependUIChildElementEvent ( pbr::windows::DIVUIElement &parent,
            pbr::windows::UIElement &element
        ) noexcept;

        ~PrependUIChildElementEvent () = default;

        void Action () noexcept;
};

} // namespace editor


#endif // EDITOR_PREPEND_UI_CHILD_ELEMENT_EVENT_HPP
