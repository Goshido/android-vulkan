#include <precompiled_headers.hpp>
#include <append_ui_child_element_event.hpp>


namespace editor {

// FUCK - remove namespace
AppendUIChildElementEvent::AppendUIChildElementEvent ( pbr::windows::DIVUIElement &parent,
    pbr::windows::UIElement &element
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
