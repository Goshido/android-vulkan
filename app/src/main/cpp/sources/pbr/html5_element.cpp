#include <precompiled_headers.hpp>
#include <pbr/html5_element.hpp>


namespace pbr {

bool HTML5Element::ApplyCSS ( char const* /*html*/, CSSParser const &/*css*/ ) noexcept
{
    // NOTHING
    return true;
}

HTML5Tag HTML5Element::GetTag () const noexcept
{
    return _tag;
}

HTML5Element::HTML5Element ( HTML5Tag::eTag tag ) noexcept:
    _tag ( tag )
{
    // NOTHING
}

} // namespace pbr
