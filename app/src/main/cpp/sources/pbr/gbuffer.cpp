#include <pbr/gbuffer.h>
#include <vulkan_utils.h>


namespace pbr {

android_vulkan::Texture2D& GBuffer::GetAlbedo () noexcept
{
    return _albedo;
}

android_vulkan::Texture2D& GBuffer::GetDepthStencil () noexcept
{
    return _depthStencil;
}

android_vulkan::Texture2D& GBuffer::GetHDRAccumulator () noexcept
{
    return _hdrAccumulator;
}

android_vulkan::Texture2D& GBuffer::GetNormal () noexcept
{
    return _normal;
}

android_vulkan::Texture2D& GBuffer::GetParams () noexcept
{
    return _params;
}

VkImageView GBuffer::GetReadOnlyDepthImageView () const noexcept
{
    return _readOnlyDepthImageView;
}

const VkExtent2D& GBuffer::GetResolution () const noexcept
{
    return _hdrAccumulator.GetResolution ();
}

bool GBuffer::Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution ) noexcept
{
    constexpr VkImageUsageFlags usageColor = AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT );

    if ( !_albedo.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_SRGB, usageColor, renderer ) )
        return false;

    VkDevice device = renderer.GetDevice ();

    if ( !_normal.CreateRenderTarget ( resolution, VK_FORMAT_A2R10G10B10_UNORM_PACK32, usageColor, renderer ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_params.CreateRenderTarget ( resolution, VK_FORMAT_R8G8B8A8_UNORM, usageColor, renderer ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkImageUsageFlags usageAccumulator = AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT );

    if ( !_hdrAccumulator.CreateRenderTarget ( resolution, VK_FORMAT_R16G16B16A16_SFLOAT, usageAccumulator, renderer ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkImageUsageFlags usageDepthStencil = AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT );

    bool result = _depthStencil.CreateRenderTarget ( resolution,
        renderer.GetDefaultDepthStencilFormat (),
        usageDepthStencil,
        renderer
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    VkImageViewCreateInfo const imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = _depthStencil.GetImage (),
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = _depthStencil.GetFormat (),

        .components
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateImageView ( device, &imageInfo, nullptr, &_readOnlyDepthImageView ),
        "pbr::GBuffer::Init",
        "Can't create read only depth image view"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_IMAGE_VIEW ( "pbr::GBuffer::_readOnlyDepthImageView" )
    return true;
}

void GBuffer::Destroy ( VkDevice device ) noexcept
{
    if ( _readOnlyDepthImageView != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _readOnlyDepthImageView, nullptr );
        _readOnlyDepthImageView = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "pbr::GBuffer::_readOnlyDepthImageView" )
    }

    _depthStencil.FreeResources ( device );
    _params.FreeResources ( device );
    _normal.FreeResources ( device );
    _hdrAccumulator.FreeResources ( device );
    _albedo.FreeResources ( device );
}

} // namespace pbr
