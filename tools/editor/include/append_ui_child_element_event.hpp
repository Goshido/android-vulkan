#ifndef EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
#define EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP


// FUCK - remove namespace
#include <platform/android/pbr/div_ui_element.hpp>
#include <platform/windows/pbr/div_ui_element.hpp>


namespace editor {

class AppendUIChildElementEvent final
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
        AppendUIChildElementEvent () = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent const & ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent const & ) = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent && ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent && ) = delete;

        // FUCK - remove it
        explicit AppendUIChildElementEvent ( pbr::android::DIVUIElement &parent,
            pbr::android::UIElement &element
        ) noexcept;

        // FUCK - remove namespace
        explicit AppendUIChildElementEvent ( pbr::windows::DIVUIElement &parent,
            pbr::windows::UIElement &element
        ) noexcept;

        ~AppendUIChildElementEvent () = default;

        void Action () noexcept;
};

} // namespace editor


#endif // EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
