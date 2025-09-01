#ifndef EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
#define EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP


// FUCK - remove namespace
#include <platform/android/pbr/div_ui_element.hpp>


namespace editor {

class AppendUIChildElementEvent final
{
    private:
        // FUCK - remove namespace
        pbr::android::UIElement         &_element;
        pbr::android::DIVUIElement      &_parent;

    public:
        AppendUIChildElementEvent () = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent const & ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent const & ) = delete;

        AppendUIChildElementEvent ( AppendUIChildElementEvent && ) = delete;
        AppendUIChildElementEvent &operator = ( AppendUIChildElementEvent && ) = delete;

        // FUCK - remove namespace
        explicit AppendUIChildElementEvent ( pbr::android::DIVUIElement &parent,
            pbr::android::UIElement &element
        ) noexcept;

        ~AppendUIChildElementEvent () = default;

        void Action () noexcept;
};

} // namespace editor


#endif // EDITOR_APPEND_UI_CHILD_ELEMENT_EVENT_HPP
