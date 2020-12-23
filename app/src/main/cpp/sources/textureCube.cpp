#include <textureCube.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint32_t const VULKAN_TEXTURE_CUBE_LAYERS = 6U;

TextureCube::TextureCube () noexcept:
    _format ( VK_FORMAT_UNDEFINED ),
    _image ( VK_NULL_HANDLE ),
    _imageDeviceMemory ( VK_NULL_HANDLE ),
    _imageView ( VK_NULL_HANDLE ),
    _mipLevels ( 0U ),
    _resolution { .width = 0U, .height = 0U }
{
    // NOTHING
}

[[maybe_unused]] bool TextureCube::CreateRenderTarget ( VkExtent2D const &resolution,
    VkFormat format,
    VkImageUsageFlags usage,
    Renderer &renderer
)
{
    VkImageCreateInfo imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;

    VkExtent3D& ext = imageInfo.extent;
    ext.width = resolution.width;
    ext.height = resolution.height;
    ext.depth = 1U;

    imageInfo.mipLevels = 1U;
    imageInfo.arrayLayers = VULKAN_TEXTURE_CUBE_LAYERS;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0U;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult ( vkCreateImage ( device, &imageInfo, nullptr, &_image ),
        "TextureCube::CreateRenderTarget",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "TextureCube::_image" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _image, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _imageDeviceMemory,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate image memory (TextureCube::CreateRenderTarget)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "TextureCube::_imageDeviceMemory" )

    result = Renderer::CheckVkResult ( vkBindImageMemory ( device, _image, _imageDeviceMemory, 0U ),
        "TextureCube::CreateRenderTarget",
        "Can't bind image memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkImageViewCreateInfo  viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0U;
    viewInfo.image = _image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = format;

    VkComponentMapping& mapping = viewInfo.components;
    mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageSubresourceRange& sub = viewInfo.subresourceRange;
    sub.aspectMask = Renderer::ResolveImageViewAspect ( format );
    sub.baseMipLevel = 0U;
    sub.levelCount = 1U;
    sub.baseArrayLayer = 0U;
    sub.layerCount = VULKAN_TEXTURE_CUBE_LAYERS;

    result = Renderer::CheckVkResult ( vkCreateImageView ( device, &viewInfo, nullptr, &_imageView ),
        "TextureCube::CreateRenderTarget",
        "Can't create image view"
    );

    if ( result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_IMAGE_VIEW ( "TextureCube::_imageView" )

    _format = format;
    _mipLevels = 1U;
    _resolution = resolution;

    return true;
}

[[maybe_unused]] void TextureCube::FreeResources ( Renderer &renderer )
{
    _format = VK_FORMAT_UNDEFINED;
    _mipLevels = 0U;
    memset ( &_resolution, 0, sizeof ( _resolution ) );

    VkDevice device = renderer.GetDevice ();

    if ( _imageView != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _imageView, nullptr );
        _imageView = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "TextureCube::_imageView" )
    }

    if ( _imageDeviceMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _imageDeviceMemory, nullptr );
        _imageDeviceMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "TextureCube::_imageDeviceMemory" )
    }

    if ( _image == VK_NULL_HANDLE )
        return;

    vkDestroyImage ( device, _image, nullptr );
    _image = VK_NULL_HANDLE;
    AV_UNREGISTER_IMAGE ( "TextureCube::_image" )
}

[[maybe_unused]] VkFormat TextureCube::GetFormat () const
{
    assert ( _format != VK_FORMAT_UNDEFINED );
    return _format;
}

[[maybe_unused]] VkImage TextureCube::GetImage () const
{
    return _image;
}

[[maybe_unused]] VkImageView TextureCube::GetImageView () const
{
    return _imageView;
}

[[maybe_unused]] uint8_t TextureCube::GetMipLevelCount () const
{
    return _mipLevels;
}

[[maybe_unused]] VkExtent2D const& TextureCube::GetResolution () const
{
    return _resolution;
}

} // namespace android_vulkan
