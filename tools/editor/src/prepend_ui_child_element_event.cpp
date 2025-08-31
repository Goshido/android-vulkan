#include <precompiled_headers.hpp>
#include <prepend_ui_child_element_event.hpp>


namespace editor {

PrependUIChildElementEvent::PrependUIChildElementEvent ( pbr::android::DIVUIElement &parent,
    pbr::UIElement &element
) noexcept:
    _element ( element ),
    _parent ( parent )
{
    // NOTHING
}

void PrependUIChildElementEvent::Action () noexcept
{
    _parent.PrependChildElement ( _element );
}

} // namespace editor
