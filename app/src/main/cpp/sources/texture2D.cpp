#include <texture2D.h>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cassert>
#include <regex>
#include <set>
#include <thread>
#include <unordered_map>

#define STBI_MALLOC(sz) std::malloc(sz)
#define STBI_REALLOC(p, newsz) std::realloc(p, newsz)
#define STBI_FREE(p) std::free(p)
#define STBI_NO_FAILURE_STRINGS
#define STBI_NEON
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_HDR
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

GX_RESTORE_WARNING_STATE

#include <file.h>
#include <ktx_media_container.h>
#include <logger.h>
#include <vulkan_utils.h>


namespace android_vulkan {

constexpr static const size_t EXPANDER_THREADS = 4U;
constexpr static const size_t RGB_BYTES_PER_PIXEL = 3U;
constexpr static const size_t RGBA_BYTES_PER_PIXEL = 4U;

constexpr static VkImageUsageFlags const IMMUTABLE_TEXTURE_USAGE = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) |
    AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_DST_BIT ) |
    AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_SRC_BIT );

static std::unordered_map<VkFormat, VkFormat> const g_FormatMapper =
{
    { VK_FORMAT_ASTC_6x6_UNORM_BLOCK, VK_FORMAT_ASTC_6x6_UNORM_BLOCK },
    { VK_FORMAT_ASTC_6x6_SRGB_BLOCK, VK_FORMAT_ASTC_6x6_SRGB_BLOCK },
    { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB }
};

//----------------------------------------------------------------------------------------------------------------------

bool Texture2D::CreateRenderTarget ( VkExtent2D const &resolution,
    VkFormat format,
    VkImageUsageFlags usage,
    Renderer &renderer
) noexcept
{
    FreeResources ( renderer.GetDevice () );
    VkImageCreateInfo imageInfo;

    if ( !CreateCommonResources ( imageInfo, resolution, format, usage, 1U, renderer ) )
        return false;

    _mipLevels = 1U;
    return true;
}

void Texture2D::FreeResources ( VkDevice device ) noexcept
{
    FreeTransferResources ( device );
    FreeResourceInternal ( device );

    _format = VK_FORMAT_UNDEFINED;
    memset ( &_resolution, 0, sizeof ( _resolution ) );
    _fileName.clear ();
}

void Texture2D::FreeTransferResources ( VkDevice device ) noexcept
{
    if ( _transferDeviceMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _transferDeviceMemory, nullptr );
        _transferDeviceMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "Texture2D::_transferDeviceMemory" )
    }

    if ( _transfer == VK_NULL_HANDLE )
        return;

    vkDestroyBuffer ( device, _transfer, nullptr );
    _transfer = VK_NULL_HANDLE;
    AV_UNREGISTER_BUFFER ( "Texture2D::_transfer" )
}

VkFormat Texture2D::GetFormat () const noexcept
{
    assert ( _format != VK_FORMAT_UNDEFINED );
    return _format;
}

VkImage Texture2D::GetImage () const noexcept
{
    return _image;
}

VkImageView Texture2D::GetImageView () const noexcept
{
    return _imageView;
}

uint8_t Texture2D::GetMipLevelCount () const noexcept
{
    return _mipLevels;
}

std::string const& Texture2D::GetName () const noexcept
{
    return _fileName;
}

VkExtent2D const& Texture2D::GetResolution () const noexcept
{
    return _resolution;
}

[[maybe_unused]] bool Texture2D::UploadData ( Renderer &renderer,
    std::string const &fileName,
    eFormat format,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer
) noexcept
{
    if ( fileName.empty () )
    {
        LogError ( "Texture2D::UploadData - Can't upload data. Filename is empty." );
        return false;
    }

    FreeResourceInternal ( renderer.GetDevice () );
    std::vector<uint8_t> pixelData;

    int width = 0;
    int height = 0;
    int channels = 0;

    if ( !LoadImage ( pixelData, fileName, width, height, channels ) )
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
        ResolveFormat ( actualFormat, format ),
        IMMUTABLE_TEXTURE_USAGE,
        static_cast<uint8_t> ( isGenerateMipmaps ? CountMipLevels ( resolution ) : 1U ),
        renderer
    );

    if ( !result )
        return false;

    result = UploadDataInternal ( renderer,
        pixelData.data (),
        pixelData.size (),
        isGenerateMipmaps,
        imageInfo,
        commandBuffer
    );

    if ( !result )
        return false;

    _fileName = fileName;
    return true;
}

bool Texture2D::UploadData ( Renderer &renderer,
    std::string &&fileName,
    eFormat format,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer
) noexcept
{
    if ( fileName == "textures/1x1-alpha.png" )
        GXMat3 const stop {};

    if ( fileName.empty () )
    {
        LogError ( "Texture2D::UploadData - Can't upload data. Filename is empty." );
        return false;
    }

    FreeResourceInternal ( renderer.GetDevice () );

    if ( IsCompressed ( fileName ) )
    {
        if ( !UploadCompressed ( renderer, fileName, format, commandBuffer ) )
            return false;

        _fileName = std::move ( fileName );
        return true;
    }

    std::vector<uint8_t> pixelData;

    int width = 0;
    int height = 0;
    int channels = 0;

    if ( !LoadImage ( pixelData, fileName, width, height, channels ) )
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
        ResolveFormat ( actualFormat, format ),
        IMMUTABLE_TEXTURE_USAGE,
        static_cast<uint8_t> ( isGenerateMipmaps ? CountMipLevels ( resolution ) : 1U ),
        renderer
    );

    if ( !result )
        return false;

    result = UploadDataInternal ( renderer,
        pixelData.data (),
        pixelData.size (),
        isGenerateMipmaps,
        imageInfo,
        commandBuffer
    );

    if ( !result )
        return false;

    _fileName = std::move ( fileName );
    return true;
}

bool Texture2D::UploadData ( Renderer &renderer,
    std::string_view const &fileName,
    eFormat format,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer
) noexcept
{
    return UploadData ( renderer, std::string ( fileName ), format, isGenerateMipmaps, commandBuffer );
}

bool Texture2D::UploadData ( Renderer &renderer,
    char const *fileName,
    eFormat format,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer
) noexcept
{
    return UploadData ( renderer, std::string ( fileName ), format, isGenerateMipmaps, commandBuffer );
}

bool Texture2D::UploadData ( Renderer &renderer,
    const uint8_t* data,
    size_t size,
    VkExtent2D const &resolution,
    VkFormat format,
    bool isGenerateMipmaps,
    VkCommandBuffer commandBuffer
) noexcept
{
    FreeResources ( renderer.GetDevice () );
    VkImageCreateInfo imageInfo;

    bool const result = CreateCommonResources ( imageInfo,
        resolution,
        format,
        IMMUTABLE_TEXTURE_USAGE,
        static_cast<uint8_t> ( isGenerateMipmaps ? CountMipLevels ( resolution ) : 1U ),
        renderer
    );

    if ( !result )
        return false;

    return UploadDataInternal ( renderer, data, size, isGenerateMipmaps, imageInfo, commandBuffer );
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

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0U;
    imageInfo.format = _format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.extent.width = _resolution.width;
    imageInfo.extent.height = _resolution.height;
    imageInfo.extent.depth = 1U;
    imageInfo.mipLevels = static_cast<uint32_t> ( mips );
    imageInfo.arrayLayers = 1U;
    imageInfo.queueFamilyIndexCount = 0U;
    imageInfo.pQueueFamilyIndices = nullptr;

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult ( vkCreateImage ( device, &imageInfo, nullptr, &_image ),
        "Texture2D::CreateCommonResources",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "Texture2D::_image" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _image, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _imageDeviceMemory,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate image memory (Texture2D::CreateCommonResources)"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "Texture2D::_imageDeviceMemory" )

    result = Renderer::CheckVkResult ( vkBindImageMemory ( device, _image, _imageDeviceMemory, 0U ),
        "Texture2D::CreateCommonResources",
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

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_IMAGE_VIEW ( "Texture2D::_imageView" )
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

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_BUFFER ( "Texture2D::_transfer" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _transfer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _transferDeviceMemory,
        memoryRequirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
        "Can't allocate transfer device memory (Texture2D::CreateTransferResources)"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "Texture2D::_transferDeviceMemory" )

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transfer, _transferDeviceMemory, 0U ),
        "Texture2D::CreateTransferResources",
        "Can't bind transfer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    void* destination = nullptr;

    result = Renderer::CheckVkResult (
        vkMapMemory ( device, _transferDeviceMemory, 0U, bufferInfo.size, 0U, &destination ),
        "Texture2D::CreateTransferResources",
        "Can't map transfer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    mappedBuffer = static_cast<uint8_t*> ( destination );
    return true;
}

void Texture2D::FreeResourceInternal ( VkDevice device ) noexcept
{
    _mipLevels = 0U;

    if ( _imageView != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _imageView, nullptr );
        _imageView = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "Texture2D::_imageView" )
    }

    if ( _imageDeviceMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _imageDeviceMemory, nullptr );
        _imageDeviceMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "Texture2D::_imageDeviceMemory" )
    }

    if ( _image == VK_NULL_HANDLE )
        return;

    vkDestroyImage ( device, _image, nullptr );
    _image = VK_NULL_HANDLE;
    AV_UNREGISTER_IMAGE ( "Texture2D::_image" )
}

bool Texture2D::UploadCompressed ( Renderer &renderer,
    std::string const &fileName,
    eFormat format,
    VkCommandBuffer commandBuffer
) noexcept
{
    KTXMediaContainer ktx;

    if ( !ktx.Init ( fileName ) )
        return false;

    VkImageCreateInfo imageInfo;

    bool result = CreateCommonResources ( imageInfo,
        ktx.GetMip ( 0U )._resolution,
        ResolveFormat ( ktx.GetFormat (), format ),
        IMMUTABLE_TEXTURE_USAGE,
        ktx.GetMipCount (),
        renderer
    );

    if ( !result )
        return false;

    uint8_t* mappedBuffer = nullptr;

    if ( !CreateTransferResources ( mappedBuffer, ktx.GetTotalSize (), renderer ) )
        return false;

    size_t offset = 0U;
    uint8_t const mips = ktx.GetMipCount ();

    for ( uint8_t i = 0U; i < mips; ++i )
    {
        MipInfo const& mip = ktx.GetMip ( i );
        memcpy ( mappedBuffer + offset, mip._data, static_cast<size_t> ( mip._size ) );
        offset += static_cast<size_t> ( mip._size );
    }

    VkDevice device = renderer.GetDevice ();
    vkUnmapMemory ( device, _transferDeviceMemory );

    constexpr VkCommandBufferBeginInfo const beginInfo
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
        MipInfo const& mip = ktx.GetMip ( i );

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
        "Texture2D::UploadCompressed",
        "Can't submit command"
    );

    if ( !result )
    {
        FreeResources ( device );
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
    VkCommandBuffer commandBuffer
) noexcept
{
    uint8_t* mappedBuffer = nullptr;

    if ( !CreateTransferResources ( mappedBuffer, static_cast<VkDeviceSize> ( size ), renderer ) )
        return false;

    std::memcpy ( mappedBuffer, data, size );
    VkDevice device = renderer.GetDevice ();
    vkUnmapMemory ( device, _transferDeviceMemory );

    constexpr VkCommandBufferBeginInfo const beginInfo
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

    auto isMipmapImpossible = [] ( uint32_t width, uint32_t height ) noexcept -> bool {
        return width + height < 3U;
    };

    if ( isMipmapImpossible ( imageInfo.extent.width, imageInfo.extent.height ) || !isGenerateMipmaps )
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

        result = Renderer::CheckVkResult (
            vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
            "Texture2D::UploadDataInternal",
            "Can't submit command"
        );

        if ( !result )
        {
            FreeResources ( device );
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

    VkImageBlit blitInfo {};
    blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitInfo.srcSubresource.layerCount = 1U;
    blitInfo.srcSubresource.baseArrayLayer = 0U;
    std::memset ( blitInfo.srcOffsets, 0, sizeof ( VkOffset3D ) );
    blitInfo.srcOffsets[ 1U ].z = 1;
    blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitInfo.dstSubresource.layerCount = 1U;
    blitInfo.dstSubresource.baseArrayLayer = 0U;
    std::memset ( blitInfo.dstOffsets, 0, sizeof ( VkOffset3D ) );
    blitInfo.dstOffsets[ 1U ].z = 1;

    for ( uint32_t i = 1U; i < imageInfo.mipLevels; ++i )
    {
        uint32_t const previousMip = i - 1U;
        blitInfo.srcSubresource.mipLevel = previousMip;
        blitInfo.srcOffsets[ 1U ].x = static_cast<int32_t> ( std::max ( imageInfo.extent.width >> previousMip, 1U ) );
        blitInfo.srcOffsets[ 1U ].y = static_cast<int32_t> ( std::max ( imageInfo.extent.height >> previousMip, 1U ) );
        blitInfo.dstSubresource.mipLevel = i;
        blitInfo.dstOffsets[ 1U ].x = static_cast<int32_t> ( std::max ( imageInfo.extent.width >> i, 1U ) );
        blitInfo.dstOffsets[ 1U ].y = static_cast<int32_t> ( std::max ( imageInfo.extent.height >> i, 1U ) );

        vkCmdBlitImage ( commandBuffer,
            _image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            _image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            &blitInfo,
            VK_FILTER_LINEAR
        );

        barrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrierInfo.subresourceRange.baseMipLevel = previousMip;

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

        if ( i + 1U >= imageInfo.mipLevels )
            continue;

        // There are more unprocessed mip maps. But now done with current mip map.

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

    // Note last mip must be translated to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL state.

    barrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierInfo.subresourceRange.baseMipLevel = imageInfo.mipLevels - 1U;

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

    result = Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "Texture2D::UploadDataInternal",
        "Can't submit command"
    );

    if ( !result )
    {
        FreeResources ( device );
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
    std::string const& fileName,
    int &width,
    int &height,
    int &channels
) noexcept
{
    File file ( const_cast<std::string&> ( fileName ) );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& imageContent = file.GetContent ();
    stbi_uc const* stbInData = imageContent.data ();
    auto const stbInSize = static_cast<const int> ( imageContent.size () );

    auto* imagePixels = stbi_load_from_memory ( stbInData, stbInSize, &width, &height, &channels, 0 );

    if ( channels != 3 )
    {
        auto const size =
            static_cast<size_t> ( width ) * static_cast<size_t> ( height ) * static_cast<size_t> ( channels );

        pixelData.resize ( size );
        memcpy ( pixelData.data (), imagePixels, size );
        free ( imagePixels );

        return true;
    }

    // We don't support 24bit per pixel mode.
    // Expanding up to 32bit per pixel mode...

    auto const size = static_cast<const size_t> ( RGBA_BYTES_PER_PIXEL * width * height );
    pixelData.resize ( size );
    uint8_t* dst = pixelData.data ();

    auto expander = [] ( uint32_t* dst,
        size_t dstRowSkipPixels,
        uint32_t const* nonDstMemory,
        uint8_t const* src,
        size_t srcRowSkipPixels,
        size_t rowPixelCount,
        uint32_t oneAlphaMask
    ) {
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

uint32_t Texture2D::CountMipLevels ( VkExtent2D const &resolution ) noexcept
{
    uint32_t pivot = std::max ( resolution.width, resolution.height );
    uint32_t mipCount = 1U;

    while ( pivot != 1U )
    {
        ++mipCount;
        pivot >>= 1U;
    }

    return mipCount;
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

VkFormat Texture2D::ResolveFormat ( VkFormat baseFormat, eFormat format ) noexcept
{
    if ( format == eFormat::Unorm )
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
