#include <pbr/gbuffer.h>
#include <vulkan_utils.h>


namespace pbr {

GBuffer::GBuffer ():
    _albedo {},
    _depthStencil {},
    _hdrAccumulator {},
    _normal {},
    _params {}
{
    // NOTHING
}

android_vulkan::Texture2D& GBuffer::GetAlbedo ()
{
    return _albedo;
}

android_vulkan::Texture2D& GBuffer::GetDepthStencil ()
{
    return _depthStencil;
}

android_vulkan::Texture2D& GBuffer::GetHDRAccumulator ()
{
    return _hdrAccumulator;
}

android_vulkan::Texture2D& GBuffer::GetNormal ()
{
    return _normal;
}

android_vulkan::Texture2D& GBuffer::GetParams ()
{
    return _params;
}

const VkExtent2D& GBuffer::GetResolution () const
{
    return _hdrAccumulator.GetResolution ();
}

bool GBuffer::Init ( VkExtent2D const &resolution, android_vulkan::Renderer &renderer )
{
    constexpr const VkImageUsageFlags usageColor = AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT );

    if ( !_albedo.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_SRGB, usageColor, renderer ) )
        return false;

    if ( !_hdrAccumulator.CreateRenderTarget ( resolution, VK_FORMAT_R16G16B16A16_SFLOAT, usageColor, renderer ) )
        return false;

    if ( !_normal.CreateRenderTarget ( resolution, VK_FORMAT_A2R10G10B10_UNORM_PACK32, usageColor, renderer ) )
        return false;

    if ( !_params.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_UNORM, usageColor, renderer ) )
        return false;

    constexpr VkImageUsageFlags const usageDepthStencil = AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT );

    return _depthStencil.CreateRenderTarget ( resolution,
        renderer.GetDefaultDepthStencilFormat (),
        usageDepthStencil,
        renderer
    );
}

void GBuffer::Destroy ( android_vulkan::Renderer &renderer )
{
    _depthStencil.FreeResources ( renderer );
    _params.FreeResources ( renderer );
    _normal.FreeResources ( renderer );
    _hdrAccumulator.FreeResources ( renderer );
    _albedo.FreeResources ( renderer );
}

} // namespace pbr
