#include <precompiled_headers.hpp>
#include <append_ui_child_element_event.hpp>


namespace editor {

AppendUIChildElementEvent::AppendUIChildElementEvent ( pbr::android::DIVUIElement &parent,
    pbr::UIElement &element
) noexcept:
    _element ( element ),
    _parent ( parent )
{
    // NOTHING
}

void AppendUIChildElementEvent::Action () noexcept
{
    _parent.AppendChildElement ( _element );
}

} // namespace editor
