#include <precompiled_headers.hpp>
#include <texture2D.hpp>

GX_DISABLE_COMMON_WARNINGS

#define STBI_MALLOC(sz) std::malloc(sz)
#define STBI_REALLOC(p, newsz) std::realloc(p, newsz)
#define STBI_FREE(p) std::free(p)
#define STBI_NO_FAILURE_STRINGS
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_HDR
#define STB_IMAGE_IMPLEMENTATION

#ifdef __ARM_NEON

#define STBI_NEON

#endif // __ARM_NEON

#include <stb/stb_image.h>

GX_RESTORE_WARNING_STATE

#include <file.hpp>
#include <ktx_media_container.hpp>
#include <logger.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

namespace {

constexpr const size_t EXPANDER_THREADS = 4U;
constexpr const size_t RGB_BYTES_PER_PIXEL = 3U;
constexpr const size_t RGBA_BYTES_PER_PIXEL = 4U;

constexpr VkImageUsageFlags IMMUTABLE_TEXTURE_USAGE = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) |
    AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_DST_BIT ) |
    AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_SRC_BIT );

std::unordered_map<VkFormat, VkFormat> const g_FormatMapper =
{
    { VK_FORMAT_ASTC_6x6_UNORM_BLOCK, VK_FORMAT_ASTC_6x6_UNORM_BLOCK },
    { VK_FORMAT_ASTC_6x6_SRGB_BLOCK, VK_FORMAT_ASTC_6x6_SRGB_BLOCK },
    { VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SRGB },
    { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void Texture2D::AssignName ( std::string &&name ) noexcept
{
    _fileName = std::move ( name );
}

bool Texture2D::CreateRenderTarget ( VkExtent2D const &resolution,
    VkFormat format,
    VkImageUsageFlags usage,
    Renderer &renderer
) noexcept
{
    FreeResources ( renderer );
    VkImageCreateInfo imageInfo;

    if ( !CreateCommonResources ( imageInfo, resolution, format, usage, 1U, renderer ) ) [[unlikely]]
        return false;

    _mipLevels = 1U;
    return true;
}

void Texture2D::FreeResources ( Renderer &renderer ) noexcept
{
    FreeTransferResources ( renderer );
    FreeResourceInternal ( renderer );

    _format = VK_FORMAT_UNDEFINED;
    std::memset ( &_resolution, 0, sizeof ( _resolution ) );
    _fileName.clear ();
}

void Texture2D::FreeTransferResources ( Renderer &renderer ) noexcept
{
    if ( _transfer != VK_NULL_HANDLE ) [[unlikely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _transfer, VK_NULL_HANDLE ), nullptr );

    if ( _transferDeviceMemory == VK_NULL_HANDLE ) [[likely]]
        return;

    renderer.FreeMemory ( std::exchange ( _transferDeviceMemory, VK_NULL_HANDLE ), _transferMemoryOffset );
    _transferMemoryOffset = 0U;
}

VkFormat Texture2D::GetFormat () const noexcept
{
    assert ( _format != VK_FORMAT_UNDEFINED );
    return _format;
}

[[maybe_unused]] VkImage const &Texture2D::GetImage () const noexcept
{
    return _image;
}

VkImageView const &Texture2D::GetImageView () const noexcept
{
    return _imageView;
}

uint8_t Texture2D::GetMipLevelCount () const noexcept
{
    return _mipLevels;
}

std::string const &Texture2D::GetName () const noexcept
{
    return _fileName;
}

bool Texture2D::IsInit () const noexcept
{
    return _format != VK_FORMAT_UNDEFINED;
}

VkExtent2D const &Texture2D::GetResolution () const noexcept
{
    return _resolution;
}

bool Texture2D::UploadData ( Renderer &renderer,
    std::string const &fileName,
    eColorSpace space,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    if ( fileName.empty () ) [[unlikely]]
    {
        LogError ( "Texture2D::UploadData - Can't upload data. Filename is empty." );
        return false;
    }

    FreeResourceInternal ( renderer );
    std::vector<uint8_t> pixelData;

    int width = 0;
    int height = 0;
    int channels = 0;

    if ( !LoadImage ( pixelData, fileName, width, height, channels ) ) [[unlikely]]
        return false;

    const VkFormat actualFormat = PickupFormat ( channels );
    VkImageCreateInfo imageInfo;

    VkExtent2D const resolution
    {
        .width = static_cast<uint32_t> ( width ),
        .height = static_cast<uint32_t> ( height )
    };

    bool result = CreateCommonResources ( imageInfo,
        resolution,
        ResolveFormat ( actualFormat, space ),
        IMMUTABLE_TEXTURE_USAGE,
        isGenerateMipmaps ? CountMipLevels ( resolution ) : UINT8_C ( 1U ),
        renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    result = UploadDataInternal ( renderer,
        pixelData.data (),
        pixelData.size (),
        isGenerateMipmaps,
        imageInfo,
        commandBuffer,
        fence
    );

    if ( !result ) [[unlikely]]
        return false;

    _fileName = fileName;
    return true;
}

bool Texture2D::UploadData ( Renderer &renderer,
    std::string &&fileName,
    eColorSpace space,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    if ( fileName.empty () ) [[unlikely]]
    {
        LogError ( "Texture2D::UploadData - Can't upload data. Filename is empty." );
        return false;
    }

    FreeResourceInternal ( renderer );

    if ( IsCompressed ( fileName ) )
    {
        if ( !UploadCompressed ( renderer, fileName, commandBuffer, fence ) ) [[unlikely]]
            return false;

        _fileName = std::move ( fileName );
        return true;
    }

    std::vector<uint8_t> pixelData;

    int width = 0;
    int height = 0;
    int channels = 0;

    if ( !LoadImage ( pixelData, fileName, width, height, channels ) ) [[unlikely]]
        return false;

    VkFormat const actualFormat = PickupFormat ( channels );
    VkImageCreateInfo imageInfo;

    VkExtent2D const resolution
    {
        .width = static_cast<uint32_t> ( width ),
        .height = static_cast<uint32_t> ( height )
    };

    bool result = CreateCommonResources ( imageInfo,
        resolution,
        ResolveFormat ( actualFormat, space ),
        IMMUTABLE_TEXTURE_USAGE,
        isGenerateMipmaps ? CountMipLevels ( resolution ) : UINT8_C ( 1U ),
        renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    result = UploadDataInternal ( renderer,
        pixelData.data (),
        pixelData.size (),
        isGenerateMipmaps,
        imageInfo,
        commandBuffer,
        fence
    );

    if ( !result ) [[unlikely]]
        return false;

    _fileName = std::move ( fileName );
    return true;
}

bool Texture2D::UploadData ( Renderer &renderer,
    std::string_view const &fileName,
    eColorSpace space,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    return UploadData ( renderer, std::string ( fileName ), space, isGenerateMipmaps, commandBuffer, fence );
}

bool Texture2D::UploadData ( Renderer &renderer,
    char const *fileName,
    eColorSpace space,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    return UploadData ( renderer, std::string ( fileName ), space, isGenerateMipmaps, commandBuffer, fence );
}

bool Texture2D::UploadData ( Renderer &renderer,
    const uint8_t* data,
    size_t size,
    VkExtent2D const &resolution,
    VkFormat format,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    FreeResources ( renderer );
    VkImageCreateInfo imageInfo;

    bool const result = CreateCommonResources ( imageInfo,
        resolution,
        format,
        IMMUTABLE_TEXTURE_USAGE,
        isGenerateMipmaps ? CountMipLevels ( resolution ) : UINT8_C ( 1U ),
        renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    return UploadDataInternal ( renderer, data, size, isGenerateMipmaps, imageInfo, commandBuffer, fence );
}

uint8_t Texture2D::CountMipLevels ( VkExtent2D const &resolution ) noexcept
{
    return static_cast<uint8_t> ( std::bit_width ( std::max ( resolution.width, resolution.height ) ) );
}

bool Texture2D::CreateCommonResources ( VkImageCreateInfo &imageInfo,
    VkExtent2D const &resolution,
    VkFormat format,
    VkImageUsageFlags usage,
    uint8_t mips,
    Renderer &renderer
) noexcept
{
    _format = format;
    _resolution = resolution;

    imageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = _format,

        .extent
        {
            .width = _resolution.width,
            .height = _resolution.height,
            .depth = 1U
        },

        .mipLevels = static_cast<uint32_t> ( mips ),
        .arrayLayers = 1U,
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
        "Texture2D::CreateCommonResources",
        "Can't create image"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _image, VK_OBJECT_TYPE_IMAGE, "Image2D" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _image, &memoryRequirements );

    constexpr VkMemoryPropertyFlags const cases[] =
    {
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT )
    };

    constexpr auto trans = static_cast<uint32_t> ( VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT );

    result = renderer.TryAllocateMemory ( _imageDeviceMemory,
        _imageMemoryOffset,
        memoryRequirements,
        cases[ static_cast<size_t> ( ( usage & trans ) == trans ) ],
        "Can't allocate image memory (Texture2D::CreateCommonResources)"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    result = Renderer::CheckVkResult ( vkBindImageMemory ( device, _image, _imageDeviceMemory, _imageMemoryOffset ),
        "Texture2D::CreateCommonResources",
        "Can't bind image memory"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    VkImageViewCreateInfo const viewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = _image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = _format,

        .components
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },

        .subresourceRange
        {
            .aspectMask = Renderer::ResolveImageViewAspect ( _format ),
            .baseMipLevel = 0U,
            .levelCount = imageInfo.mipLevels,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    result = Renderer::CheckVkResult ( vkCreateImageView ( device, &viewInfo, nullptr, &_imageView ),
        "Texture2D::CreateCommonResources",
        "Can't create image view"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    AV_SET_VULKAN_OBJECT_NAME ( device, _imageView, VK_OBJECT_TYPE_IMAGE_VIEW, "Image2D" )
    return true;
}

bool Texture2D::CreateTransferResources ( uint8_t* &mappedBuffer, VkDeviceSize size, Renderer &renderer ) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transfer ),
        "Texture2D::CreateTransferResources",
        "Can't create transfer buffer"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    AV_SET_VULKAN_OBJECT_NAME ( device, _transfer, VK_OBJECT_TYPE_BUFFER, "Texture2D staging" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _transfer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _transferDeviceMemory,
        _transferMemoryOffset,
        memoryRequirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
        "Can't allocate transfer device memory (Texture2D::CreateTransferResources)"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transfer, _transferDeviceMemory, _transferMemoryOffset ),
        "Texture2D::CreateTransferResources",
        "Can't bind transfer memory"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    void* destination = nullptr;

    result = renderer.MapMemory ( destination,
        _transferDeviceMemory,
        _transferMemoryOffset,
        "Texture2D::CreateTransferResources",
        "Can't map transfer memory"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    mappedBuffer = static_cast<uint8_t*> ( destination );
    return true;
}

void Texture2D::FreeResourceInternal ( Renderer &renderer ) noexcept
{
    _mipLevels = 0U;
    VkDevice device = renderer.GetDevice ();

    if ( _imageView != VK_NULL_HANDLE )
        vkDestroyImageView ( device, std::exchange ( _imageView, VK_NULL_HANDLE ), nullptr );

    if ( _image != VK_NULL_HANDLE )
        vkDestroyImage ( device, std::exchange ( _image, VK_NULL_HANDLE ), nullptr );

    if ( _imageDeviceMemory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( std::exchange ( _imageDeviceMemory, VK_NULL_HANDLE ), _imageMemoryOffset );
    _imageMemoryOffset = 0U;
}

bool Texture2D::UploadCompressed ( Renderer &renderer,
    std::string const &fileName,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    KTXMediaContainer ktx{};

    if ( !ktx.Init ( fileName ) ) [[unlikely]]
        return false;

    // Note color space is defined in ktx container itself and can be fully trusted.
    VkImageCreateInfo imageInfo{};

    bool result = CreateCommonResources ( imageInfo,
        ktx.GetMip ( 0U )._resolution,
        ktx.GetFormat (),
        IMMUTABLE_TEXTURE_USAGE,
        ktx.GetMipCount (),
        renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    uint8_t* mappedBuffer = nullptr;

    if ( !CreateTransferResources ( mappedBuffer, ktx.GetTotalSize (), renderer ) ) [[unlikely]]
        return false;

    size_t offset = 0U;
    uint8_t const mips = ktx.GetMipCount ();

    for ( uint8_t i = 0U; i < mips; ++i )
    {
        MipInfo const &mip = ktx.GetMip ( i );
        std::memcpy ( mappedBuffer + offset, mip._data, static_cast<size_t> ( mip._size ) );
        offset += static_cast<size_t> ( mip._size );
    }

    renderer.UnmapMemory ( _transferDeviceMemory );

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ),
        "Texture2D::UploadCompressed",
        "Can't begin command buffer"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
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
            .layerCount = 1U
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
    copyRegion.imageOffset.x = 0;
    copyRegion.imageOffset.y = 0;
    copyRegion.imageOffset.z = 0;
    copyRegion.imageExtent.depth = 1U;
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.layerCount = 1U;
    copyRegion.imageSubresource.baseArrayLayer = 0U;
    copyRegion.bufferRowLength = 0U;
    copyRegion.bufferImageHeight = 0U;

    offset = 0U;

    for ( uint8_t i = 0U; i < mips; ++i )
    {
        MipInfo const &mip = ktx.GetMip ( i );

        copyRegion.imageSubresource.mipLevel = static_cast<uint32_t> ( i );
        copyRegion.imageExtent.width = mip._resolution.width;
        copyRegion.imageExtent.height = mip._resolution.height;
        copyRegion.bufferOffset = static_cast<VkDeviceSize> ( offset );

        vkCmdCopyBufferToImage ( commandBuffer,
            _transfer,
            _image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            &copyRegion
        );

        offset += static_cast<size_t> ( mip._size );
    }

    barrierInfo.subresourceRange.levelCount = static_cast<uint32_t> ( mips );
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
        "Texture2D::UploadCompressed",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
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

    result = Renderer::CheckVkResult ( vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fence ),
        "Texture2D::UploadCompressed",
        "Can't submit command"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    _mipLevels = mips;
    return true;
}

bool Texture2D::UploadDataInternal ( Renderer &renderer,
    uint8_t const* data,
    size_t size,
    bool isGenerateMipmaps,
    VkImageCreateInfo const &imageInfo,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    uint8_t* mappedBuffer = nullptr;

    if ( !CreateTransferResources ( mappedBuffer, static_cast<VkDeviceSize> ( size ), renderer ) ) [[unlikely]]
        return false;

    std::memcpy ( mappedBuffer, data, size );
    renderer.UnmapMemory ( _transferDeviceMemory );

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ),
        "Texture2D::UploadDataInternal",
        "Can't begin command buffer"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
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
            .levelCount = imageInfo.mipLevels,
            .baseArrayLayer = 0U,
            .layerCount = 1U
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

    VkBufferImageCopy const copyRegion
    {
        .bufferOffset = 0U,
        .bufferRowLength = imageInfo.extent.width,
        .bufferImageHeight = imageInfo.extent.height,

        .imageSubresource
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        },

        .imageOffset
        {
            .x = 0,
            .y = 0,
            .z = 0
        },

        .imageExtent = imageInfo.extent
    };

    vkCmdCopyBufferToImage ( commandBuffer, _transfer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &copyRegion );

    constexpr auto isMipmapImpossible = [] ( uint32_t width, uint32_t height ) noexcept -> bool {
        return width + height < 3U;
    };

    if ( isMipmapImpossible ( imageInfo.extent.width, imageInfo.extent.height ) | !isGenerateMipmaps )
    {
        barrierInfo.subresourceRange.levelCount = 1U;
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
            "Texture2D::UploadDataInternal",
            "Can't end command buffer"
        );

        if ( !result ) [[unlikely]]
        {
            FreeResources ( renderer );
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

        result = Renderer::CheckVkResult (
            vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fence ),
            "Texture2D::UploadDataInternal",
            "Can't submit command"
        );

        if ( !result ) [[unlikely]]
        {
            FreeResources ( renderer );
            return false;
        }

        _mipLevels = static_cast<uint8_t> ( imageInfo.mipLevels );
        return true;
    }

    barrierInfo.subresourceRange.levelCount = 1U;
    barrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrierInfo
    );

    VkImageBlit blitInfo
    {
        .srcSubresource
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        },

        .srcOffsets
        {
            {
                .x = 0,
                .y = 0,
                .z = 0
            },

            {
                .x = 0,
                .y = 0,
                .z = 1
            }
        },

        .dstSubresource
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        },

        .dstOffsets
        {
            {
                .x = 0,
                .y = 0,
                .z = 0
            },

            {
                .x = 0,
                .y = 0,
                .z = 1
            }
        }
    };

    VkOffset3D &src = blitInfo.srcOffsets[ 1U ];
    VkOffset3D &dst = blitInfo.dstOffsets[ 1U ];

    for ( uint32_t i = 1U; i < imageInfo.mipLevels; ++i )
    {
        uint32_t const previousMip = i - 1U;

        blitInfo.srcSubresource.mipLevel = previousMip;
        src.x = static_cast<int32_t> ( std::max ( imageInfo.extent.width >> previousMip, 1U ) );
        src.y = static_cast<int32_t> ( std::max ( imageInfo.extent.height >> previousMip, 1U ) );

        blitInfo.dstSubresource.mipLevel = i;
        dst.x = static_cast<int32_t> ( std::max ( imageInfo.extent.width >> i, 1U ) );
        dst.y = static_cast<int32_t> ( std::max ( imageInfo.extent.height >> i, 1U ) );

        vkCmdBlitImage ( commandBuffer,
            _image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            _image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            &blitInfo,
            VK_FILTER_LINEAR
        );

        barrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrierInfo.subresourceRange.baseMipLevel = i;

        vkCmdPipelineBarrier ( commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0U,
            0U,
            nullptr,
            0U,
            nullptr,
            1U,
            &barrierInfo
        );
    }

    barrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    barrierInfo.subresourceRange.baseMipLevel = 0U;

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
        "Texture2D::UploadDataInternal",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
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

    result = Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fence ),
        "Texture2D::UploadDataInternal",
        "Can't submit command"
    );

    if ( !result ) [[unlikely]]
    {
        FreeResources ( renderer );
        return false;
    }

    _mipLevels = static_cast<uint8_t> ( imageInfo.mipLevels );
    return true;
}

bool Texture2D::IsCompressed ( std::string const &fileName ) noexcept
{
    static std::regex const isKTX ( R"__(^.+\.(?:ktx|KTX)$)__" );
    std::smatch match;
    return std::regex_match ( fileName, match, isKTX );
}

bool Texture2D::LoadImage ( std::vector<uint8_t> &pixelData,
    std::string const &fileName,
    int &width,
    int &height,
    int &channels
) noexcept
{
    File file ( const_cast<std::string &> ( fileName ) );

    if ( !file.LoadContent () ) [[unlikely]]
        return false;

    std::vector<uint8_t> const &imageContent = file.GetContent ();
    stbi_uc const* stbInData = imageContent.data ();
    auto const stbInSize = static_cast<const int> ( imageContent.size () );

    auto* imagePixels = stbi_load_from_memory ( stbInData, stbInSize, &width, &height, &channels, 0 );

    if ( channels != 3 )
    {
        auto const size =
            static_cast<size_t> ( width ) * static_cast<size_t> ( height ) * static_cast<size_t> ( channels );

        pixelData.resize ( size );
        std::memcpy ( pixelData.data (), imagePixels, size );
        free ( imagePixels );

        return true;
    }

    // We don't support 24bit per pixel mode.
    // Expanding up to 32bit per pixel mode...

    auto const size = static_cast<const size_t> ( RGBA_BYTES_PER_PIXEL * width * height );
    pixelData.resize ( size );
    uint8_t* dst = pixelData.data ();

    constexpr auto expander = [] ( uint32_t* dst,
        size_t dstRowSkipPixels,
        uint32_t const* nonDstMemory,
        uint8_t const* src,
        size_t srcRowSkipPixels,
        size_t rowPixelCount,
        uint32_t oneAlphaMask
    ) noexcept {
        while ( dst < nonDstMemory )
        {
            for ( size_t i = 0U; i < rowPixelCount; ++i )
            {
                *dst = *reinterpret_cast<uint32_t const*> ( src );
                *dst |= oneAlphaMask;

                ++dst;
                src += RGB_BYTES_PER_PIXEL;
            }

            src += srcRowSkipPixels;
            dst += dstRowSkipPixels;
        }
    };

    constexpr size_t const SKIP_ROWS = EXPANDER_THREADS - 1U;

    auto const rowPixelCount = static_cast<size_t const> ( width );
    auto const srcRowSize = RGB_BYTES_PER_PIXEL * rowPixelCount;
    auto const srcRowSkipPixels = SKIP_ROWS * srcRowSize;
    auto const dstRowSize = RGBA_BYTES_PER_PIXEL * rowPixelCount;
    auto const dstRowSkipPixels = SKIP_ROWS * rowPixelCount;
    auto const* nonDstMemory = reinterpret_cast<uint32_t const*> ( dst + size );

    constexpr uint8_t const oneAlphaRaw[ RGBA_BYTES_PER_PIXEL ] = { 0x00U, 0x00U, 0x00U, 0xFFU };
    auto const oneAlphaMask = *reinterpret_cast<uint32_t const*> ( oneAlphaRaw );

    std::array<std::thread, EXPANDER_THREADS> expanders;

    for ( size_t i = 0U; i < EXPANDER_THREADS; ++i )
    {
        expanders[ i ] = std::thread ( expander,
            reinterpret_cast<uint32_t*> ( dst + i * dstRowSize ),
            dstRowSkipPixels,
            nonDstMemory,
            imagePixels + i * srcRowSize,
            srcRowSkipPixels,
            rowPixelCount,
            oneAlphaMask
        );
    }

    for ( size_t i = 0U; i < EXPANDER_THREADS; ++i )
        expanders[ i ].join ();

    STBI_FREE ( imagePixels );
    channels = static_cast<int> ( RGBA_BYTES_PER_PIXEL );

    return true;
}

VkFormat Texture2D::PickupFormat ( int channels ) noexcept
{
    switch ( channels )
    {
        case 1:
        return VK_FORMAT_R8_UNORM;

        case 2:
        return VK_FORMAT_R8G8_UNORM;

        case 3:
            LogError ( "Texture2D::PickupFormat - Three channel formats are not supported!" );
        return VK_FORMAT_UNDEFINED;

        case 4:
            return VK_FORMAT_R8G8B8A8_UNORM;

        default:
            LogError (
                "Texture2D::PickupFormat - Unexpected channel count: %i! Supported channel count: 1, 2 or 4."
            );
        return VK_FORMAT_UNDEFINED;
    }
}

VkFormat Texture2D::ResolveFormat ( VkFormat baseFormat, eColorSpace space ) noexcept
{
    if ( space == eColorSpace::Unorm )
        return baseFormat;

    auto const findResult = g_FormatMapper.find ( baseFormat );
    assert ( findResult != g_FormatMapper.cend () );

    if ( findResult != g_FormatMapper.cend () )
        return findResult->second;

    LogError ( "Texture2D::ResolveFormat - Can't find sRGB pair for %s format.",
        Renderer::ResolveVkFormat ( baseFormat )
    );

    return VK_FORMAT_UNDEFINED;
}

} // namespace android_vulkan
