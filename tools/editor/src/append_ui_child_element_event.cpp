#include <precompiled_headers.hpp>
#include <append_ui_child_element_event.hpp>


namespace editor {

// FUCK - remove it
namespace {

pbr::android::DIVUIElement null ( nullptr, {} );
pbr::windows::DIVUIElement nullEXT ( nullptr, {} );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

// FUCK - remove it
AppendUIChildElementEvent::AppendUIChildElementEvent ( pbr::android::DIVUIElement &parent,
    pbr::android::UIElement &element
) noexcept:
    _element ( element ),
    _elementEXT ( nullEXT ),
    _parent ( parent ),
    _parentEXT ( nullEXT ),
    _isEXT ( false )
{
    // NOTHING
}

// FUCK - remove namespace
AppendUIChildElementEvent::AppendUIChildElementEvent ( pbr::windows::DIVUIElement &parent,
    pbr::windows::UIElement &element
) noexcept:
    _element ( null ),
    _elementEXT ( element ),
    _parent ( null ),
    _parentEXT ( parent ),
    _isEXT ( true )
{
    // NOTHING
}

void AppendUIChildElementEvent::Action () noexcept
{
    if ( !_isEXT ) [[likely]]
    {
        _parent.AppendChildElement ( _element );
        return;
    }

    _parentEXT.AppendChildElement ( _elementEXT );
}

} // namespace editor
