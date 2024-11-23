#include <precompiled_headers.hpp>
#include <pbr/gbuffer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

android_vulkan::Texture2D &GBuffer::GetAlbedo () noexcept
{
    return _albedo;
}

android_vulkan::Texture2D &GBuffer::GetDepthStencil () noexcept
{
    return _depthStencil;
}

android_vulkan::Texture2D &GBuffer::GetHDRAccumulator () noexcept
{
    return _hdrAccumulator;
}

android_vulkan::Texture2D &GBuffer::GetNormal () noexcept
{
    return _normal;
}

android_vulkan::Texture2D &GBuffer::GetParams () noexcept
{
    return _params;
}

const VkExtent2D &GBuffer::GetResolution () const noexcept
{
    return _hdrAccumulator.GetResolution ();
}

bool GBuffer::Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution ) noexcept
{
    // [2022-08-10] RenderDoc 1.21 doesn't support VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT memory.
    // So VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT usage is not available.

#ifdef ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    constexpr VkImageUsageFlags usageColor = AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT );

    constexpr VkImageUsageFlags usageDepthStencil = AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT );

#else

    constexpr VkImageUsageFlags usageColor = AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT );

    constexpr VkImageUsageFlags usageDepthStencil = AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT );

#endif // ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    if ( !_albedo.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_SRGB, usageColor, renderer ) ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _albedo.GetImage (), VK_OBJECT_TYPE_IMAGE, "Albedo" )
    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _albedo.GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Albedo" )

    if ( !_normal.CreateRenderTarget ( resolution, VK_FORMAT_A2R10G10B10_UNORM_PACK32, usageColor, renderer ) )
    {
        [[unlikely]]
        return false;
    }

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _normal.GetImage (), VK_OBJECT_TYPE_IMAGE, "Normal" )
    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _normal.GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Normal" )

    if ( !_params.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_UNORM, usageColor, renderer ) ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _params.GetImage (), VK_OBJECT_TYPE_IMAGE, "Params" )
    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _params.GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Params" )

    constexpr VkImageUsageFlags usageAccumulator = AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT );

    if ( !_hdrAccumulator.CreateRenderTarget ( resolution, VK_FORMAT_R16G16B16A16_SFLOAT, usageAccumulator, renderer ) )
    {
        [[unlikely]]
        return false;
    }

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _hdrAccumulator.GetImage (),
        VK_OBJECT_TYPE_IMAGE,
        "HDR accumulator"
    )

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _hdrAccumulator.GetImageView (),
        VK_OBJECT_TYPE_IMAGE_VIEW,
        "HDR accumulator"
    )

    bool const result = _depthStencil.CreateRenderTarget ( resolution,
        renderer.GetDefaultDepthFormat (),
        usageDepthStencil,
        renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _depthStencil.GetImage (),
        VK_OBJECT_TYPE_IMAGE,
        "Depth"
    )

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _depthStencil.GetImageView (),
        VK_OBJECT_TYPE_IMAGE_VIEW,
        "Depth"
    )

    return true;
}

void GBuffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _depthStencil.FreeResources ( renderer );
    _params.FreeResources ( renderer );
    _normal.FreeResources ( renderer );
    _hdrAccumulator.FreeResources ( renderer );
    _albedo.FreeResources ( renderer );
}

} // namespace pbr
