#include <pbr/image_ui_element.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

ImageUIElement::ImageUIElement ( std::string &&asset, CSSComputedValues const &css ) noexcept:
    UIElement ( true ),
    _asset ( std::move ( asset ) ),
    _css ( css )
{
    // NOTHING
}

void ImageUIElement::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_ImageUIElementCollectGarbage",
            .func = &ImageUIElement::OnGarbageCollected
        },
        {
            .name = "av_ImageUIElementHide",
            .func = &ImageUIElement::OnHide
        },
        {
            .name = "av_ImageUIElementShow",
            .func = &ImageUIElement::OnShow
        }
    };

    for ( auto const& extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void ImageUIElement::ApplyLayout () noexcept
{
    // TODO
}

void ImageUIElement::Render () noexcept
{
    // TODO
}

int ImageUIElement::OnGarbageCollected ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int ImageUIElement::OnHide ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int ImageUIElement::OnShow ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
