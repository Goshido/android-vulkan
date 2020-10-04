#include <pbr/gbuffer.h>
#include <vulkan_utils.h>


namespace pbr {

GBuffer::GBuffer ():
    _albedo {},
    _emission {},
    _normal {},
    _params {},
    _depthStencil {},
    _resolution { .width = 0U, .height = 0U }
{
    // NOTHING
}

android_vulkan::Texture2D& GBuffer::GetAlbedo ()
{
    return _albedo;
}

android_vulkan::Texture2D& GBuffer::GetEmission ()
{
    return _emission;
}

android_vulkan::Texture2D& GBuffer::GetNormal ()
{
    return _normal;
}

android_vulkan::Texture2D& GBuffer::GetParams ()
{
    return _params;
}

android_vulkan::Texture2D& GBuffer::GetDepthStencil ()
{
    return _depthStencil;
}

const VkExtent2D& GBuffer::GetResolution () const
{
    return _resolution;
}

bool GBuffer::Init ( const VkExtent2D &resolution, android_vulkan::Renderer &renderer )
{
    constexpr const VkImageUsageFlags usageColor = AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT );

    if ( !_albedo.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_SRGB, usageColor, renderer ) )
        return false;

    if ( !_emission.CreateRenderTarget ( resolution, VK_FORMAT_R16G16B16A16_SFLOAT, usageColor, renderer ) )
        return false;

    if ( !_normal.CreateRenderTarget ( resolution, VK_FORMAT_A2R10G10B10_UNORM_PACK32, usageColor, renderer ) )
        return false;

    if ( !_params.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_UNORM, usageColor, renderer ) )
        return false;

    constexpr const VkImageUsageFlags usageDepthStencil = AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT );

    const bool result = _depthStencil.CreateRenderTarget ( resolution,
        renderer.GetDefaultDepthStencilFormat (),
        usageDepthStencil,
        renderer
    );

    if ( !result )
        return false;

    _resolution = resolution;
    return true;
}

void GBuffer::Destroy ( android_vulkan::Renderer &renderer )
{
    _depthStencil.FreeResources ( renderer );
    _params.FreeResources ( renderer );
    _normal.FreeResources ( renderer );
    _emission.FreeResources ( renderer );
    _albedo.FreeResources ( renderer );
}

} // namespace pbr
