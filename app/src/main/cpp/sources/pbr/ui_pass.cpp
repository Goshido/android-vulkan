#include <pbr/ui_pass.h>


namespace pbr {

bool UIPass::SetResolution ( android_vulkan::Renderer &renderer, VkRenderPass renderPass ) noexcept
{
    VkExtent2D const& resolution = renderer.GetSurfaceSize ();

    if ( ( _resolution.width == resolution.width ) & ( _resolution.height == resolution.height ) )
        return true;

    _program.Destroy ( renderer.GetDevice () );

    if ( !_program.Init ( renderer, renderPass, 0U, resolution ) )
        return false;

    _resolution = resolution;
    return true;
}

void UIPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _program.Destroy ( renderer.GetDevice () );
}

} // namespace pbr
