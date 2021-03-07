#include <texture_cube.h>
#include <ktx_media_container.h>
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
    _resolution { .width = 0U, .height = 0U },
    _transfer ( VK_NULL_HANDLE ),
    _transferDeviceMemory ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool TextureCube::CreateRenderTarget ( VkExtent2D const &resolution,
    VkFormat format,
    VkImageUsageFlags usage,
    Renderer &renderer
)
{
    VkImageCreateInfo const imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,

        .extent
        {
            .width = resolution.width,
            .height = resolution.height,
            .depth = 1U
        },

        .mipLevels = 1U,
        .arrayLayers = VULKAN_TEXTURE_CUBE_LAYERS,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

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
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "TextureCube::_imageDeviceMemory" )

    result = Renderer::CheckVkResult ( vkBindImageMemory ( device, _image, _imageDeviceMemory, 0U ),
        "TextureCube::CreateRenderTarget",
        "Can't bind image memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    VkImageViewCreateInfo const viewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = _image,
        .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
        .format = format,

        .components
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },

        .subresourceRange
        {
            .aspectMask = Renderer::ResolveImageViewAspect ( format ),
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = VULKAN_TEXTURE_CUBE_LAYERS
        }
    };

    result = Renderer::CheckVkResult ( vkCreateImageView ( device, &viewInfo, nullptr, &_imageView ),
        "TextureCube::CreateRenderTarget",
        "Can't create image view"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_IMAGE_VIEW ( "TextureCube::_imageView" )

    _format = format;
    _mipLevels = 1U;
    _resolution = resolution;

    return true;
}

bool TextureCube::UploadData ( TextureCubeData const &data )
{
    constexpr auto const sideCount = static_cast<size_t> ( VULKAN_TEXTURE_CUBE_LAYERS );

    KTXMediaContainer sides[ sideCount ];
    KTXMediaContainer& sideXPlus = sides[ 0U ];

    char const* files[ sideCount ] =
    {
        data._xPlusFile,
        data._xMinusFile,
        data._yPlusFile,
        data._yMinusFile,
        data._zPlusFile,
        data._zMinusFile
    };

    if ( !sideXPlus.Init ( files[ 0U ] ) )
        return false;

    VkDeviceSize mappedSize = sideXPlus.GetTotalSize();

    uint8_t const mips = sideXPlus.GetMipCount ();
    VkFormat const format = sideXPlus.GetFormat ();
    VkExtent2D const& resolution = sideXPlus.GetMip ( 0U )._resolution;

    for ( size_t i = 1U; i < sideCount; ++i )
    {
        KTXMediaContainer& container = sides[ i ];

        if ( !container.Init ( files[ i ] ) )
            return false;

        if ( mips != container.GetMipCount () || format != container.GetFormat () )
            return false;

        VkExtent2D const& res = container.GetMip ( 0U )._resolution;

        if ( resolution.width != res.width || resolution.height != res.height )
            return false;

        mappedSize += container.GetTotalSize ();
    }

    // TODO
    return false;
}

void TextureCube::FreeResources ( VkDevice device )
{
    _format = VK_FORMAT_UNDEFINED;
    _mipLevels = 0U;
    memset ( &_resolution, 0, sizeof ( _resolution ) );

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

VkImageView TextureCube::GetImageView () const
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
