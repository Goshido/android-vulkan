#include <texture_cube.h>
#include <ktx_media_container.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint32_t const VULKAN_TEXTURE_CUBE_LAYERS = 6U;

//----------------------------------------------------------------------------------------------------------------------

bool TextureCube::CreateRenderTarget ( Renderer &renderer,
    VkExtent2D const &resolution,
    VkFormat format,
    VkImageUsageFlags usage
) noexcept
{
    if ( !CreateImageResources ( renderer, resolution, format, usage, 1U ) )
        return false;

    _format = format;
    _mipLevels = 1U;
    _resolution = resolution;

    return true;
}

bool TextureCube::UploadData ( android_vulkan::Renderer &renderer,
    TextureCubeData const &data,
    VkCommandBuffer commandBuffer
) noexcept
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

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = mappedSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transfer ),
        "TextureCube::UploadData",
        "Can't create transfer buffer"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_BUFFER ( "TextureCube::_transfer" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _transfer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _transferDeviceMemory,
        memoryRequirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
        "Can't allocate transfer device memory (TextureCube::UploadData)"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "TextureCube::_transferDeviceMemory" )

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transfer, _transferDeviceMemory, 0U ),
        "TextureCube::UploadData",
        "Can't bind transfer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    uint8_t* destination = nullptr;

    result = Renderer::CheckVkResult (
        vkMapMemory ( device,
            _transferDeviceMemory,
            0U,
            bufferInfo.size,
            0U,
            reinterpret_cast<void**> ( &destination )
        ),

        "TextureCube::UploadData",
        "Can't map transfer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    VkDeviceSize offset = 0U;

    for ( auto const& side : sides )
    {
        for ( uint8_t mipIndex = 0U; mipIndex < mips; ++mipIndex )
        {
            MipInfo const& mip = side.GetMip ( mipIndex );
            std::memcpy ( destination + static_cast<size_t> ( offset ), mip._data, static_cast<size_t> ( mip._size ) );
            offset += mip._size;
        }
    }

    vkUnmapMemory ( device, _transferDeviceMemory );

    constexpr VkImageUsageFlags const flags = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_DST_BIT );

    if ( !CreateImageResources ( renderer, resolution, format, flags, static_cast<uint32_t> ( mips ) ) )
    {
        FreeResources ( device );
        return false;
    }

    VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ),
        "TextureCube::UploadData",
        "Can't begin command buffer"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    VkImageMemoryBarrier barrierInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0U,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = _image,

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = static_cast<uint32_t> ( mips ),
            .baseArrayLayer = 0U,
            .layerCount = VULKAN_TEXTURE_CUBE_LAYERS
        }
    };

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrierInfo
    );

    VkBufferImageCopy copyRegion {};
    copyRegion.bufferRowLength = 0U;
    copyRegion.bufferImageHeight = 0U;

    copyRegion.imageOffset =
    {
        .x = 0,
        .y = 0,
        .z = 0
    };

    VkImageSubresourceLayers& imageSubresource = copyRegion.imageSubresource;
    imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresource.layerCount = 1U;

    VkExtent3D& imageExtent = copyRegion.imageExtent;
    imageExtent.depth = 1U;

    offset = 0U;

    for ( uint32_t sideIndex = 0U; sideIndex < VULKAN_TEXTURE_CUBE_LAYERS; ++sideIndex )
    {
        imageSubresource.baseArrayLayer = sideIndex;

        for ( uint8_t mipIndex = 0U; mipIndex < mips; ++mipIndex )
        {
            imageSubresource.mipLevel = static_cast<uint32_t> ( mipIndex );

            MipInfo const& mip = sides[ sideIndex ].GetMip ( mipIndex );
            VkExtent2D const& mipRes = mip._resolution;

            imageExtent.width = mipRes.width;
            imageExtent.height = mipRes.height;

            copyRegion.bufferOffset = offset;

            vkCmdCopyBufferToImage ( commandBuffer,
                _transfer,
                _image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1U,
                &copyRegion
            );

            offset += mip._size;
        }
    }

    barrierInfo.subresourceRange.layerCount = VULKAN_TEXTURE_CUBE_LAYERS;
    barrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrierInfo
    );

    result = Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "TextureCube::UploadData",
        "Can't end command buffer"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    result = Renderer::CheckVkResult ( vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "TextureCube::UploadData",
        "Can't submit command"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    _mipLevels = mips;
    _format = format;
    _resolution = resolution;

    return true;
}

void TextureCube::FreeResources ( VkDevice device ) noexcept
{
    _format = VK_FORMAT_UNDEFINED;
    _mipLevels = 0U;
    memset ( &_resolution, 0, sizeof ( _resolution ) );

    FreeTransferResources ( device );

    if ( _imageView != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _imageView, nullptr );
        _imageView = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "TextureCube::_imageView" )
    }

    if ( _image != VK_NULL_HANDLE )
    {
        vkDestroyImage ( device, _image, nullptr );
        _image = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE ( "TextureCube::_image" )
    }

    if ( _imageDeviceMemory == VK_NULL_HANDLE )
        return;

    vkFreeMemory ( device, _imageDeviceMemory, nullptr );
    _imageDeviceMemory = VK_NULL_HANDLE;
    AV_UNREGISTER_DEVICE_MEMORY ( "TextureCube::_imageDeviceMemory" )
}

void TextureCube::FreeTransferResources ( VkDevice device ) noexcept
{
    if ( _transfer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transfer, nullptr );
        _transfer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "TextureCube::_transfer" )
    }

    if ( _transferDeviceMemory == VK_NULL_HANDLE )
        return;

    vkFreeMemory ( device, _transferDeviceMemory, nullptr );
    _transferDeviceMemory = VK_NULL_HANDLE;
    AV_UNREGISTER_DEVICE_MEMORY ( "TextureCube::_transferDeviceMemory" )
}

VkFormat TextureCube::GetFormat () const noexcept
{
    assert ( _format != VK_FORMAT_UNDEFINED );
    return _format;
}

VkImage TextureCube::GetImage () const noexcept
{
    return _image;
}

VkImageView TextureCube::GetImageView () const noexcept
{
    return _imageView;
}

[[maybe_unused]] uint8_t TextureCube::GetMipLevelCount () const noexcept
{
    return _mipLevels;
}

[[maybe_unused]] VkExtent2D const& TextureCube::GetResolution () const noexcept
{
    return _resolution;
}

[[maybe_unused]] bool TextureCube::CreateImageResources ( Renderer &renderer,
    VkExtent2D const &resolution,
    VkFormat format,
    VkImageUsageFlags usage,
    uint32_t mipLevels
) noexcept
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

        .mipLevels = mipLevels,
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
        "TextureCube::CreateImageResources",
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
        "Can't allocate image memory (TextureCube::CreateImageResources)"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "TextureCube::_imageDeviceMemory" )

    result = Renderer::CheckVkResult ( vkBindImageMemory ( device, _image, _imageDeviceMemory, 0U ),
        "TextureCube::CreateImageResources",
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
            .levelCount = mipLevels,
            .baseArrayLayer = 0U,
            .layerCount = VULKAN_TEXTURE_CUBE_LAYERS
        }
    };

    result = Renderer::CheckVkResult ( vkCreateImageView ( device, &viewInfo, nullptr, &_imageView ),
        "TextureCube::CreateImageResources",
        "Can't create image view"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_IMAGE_VIEW ( "TextureCube::_imageView" )
    return true;
}

} // namespace android_vulkan
