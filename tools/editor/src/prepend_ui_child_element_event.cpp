#include <precompiled_headers.hpp>
#include <prepend_ui_child_element_event.hpp>


namespace editor {

// FUCK - remove it
namespace {

pbr::android::DIVUIElement null ( nullptr, {} );
pbr::windows::DIVUIElement nullEXT ( nullptr, {} );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

// FUCK - remove it
PrependUIChildElementEvent::PrependUIChildElementEvent ( pbr::android::DIVUIElement &parent,
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
PrependUIChildElementEvent::PrependUIChildElementEvent ( pbr::windows::DIVUIElement &parent,
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

void PrependUIChildElementEvent::Action () noexcept
{
    if ( !_isEXT ) [[likely]]
    {
        _parent.PrependChildElement ( _element );
        return;
    }

    _parentEXT.PrependChildElement ( _elementEXT );
}

} // namespace editor
