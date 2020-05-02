#include <rotating_mesh/texture2D.h>

AV_DISABLE_COMMON_WARNINGS

#include <cassert>

#define STBI_NO_FAILURE_STRINGS
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_HDR
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

AV_RESTORE_WARNING_STATE

#include <logger.h>
#include <vulkan_utils.h>


namespace rotating_mesh {

Texture2D::Texture2D ():
    _format ( VK_FORMAT_UNDEFINED ),
    _image ( VK_NULL_HANDLE ),
    _imageDeviceMemory ( VK_NULL_HANDLE ),
    _imageView ( VK_NULL_HANDLE ),
    _resolution { .width = 0U, .height = 0U },
    _transfer ( VK_NULL_HANDLE ),
    _transferDeviceMemory ( VK_NULL_HANDLE )
{
    // NOTHING
}

Texture2D::Texture2D ( std::string &fileName, VkFormat format ):
    _format ( format ),
    _image ( VK_NULL_HANDLE ),
    _imageDeviceMemory ( VK_NULL_HANDLE ),
    _imageView ( VK_NULL_HANDLE ),
    _resolution { .width = 0U, .height = 0U },
    _transfer ( VK_NULL_HANDLE ),
    _transferDeviceMemory ( VK_NULL_HANDLE ),
    _fileName ( fileName )
{
    // NOTHING
}

Texture2D::Texture2D ( std::string &&fileName, VkFormat format ):
    _format ( format ),
    _image ( VK_NULL_HANDLE ),
    _imageDeviceMemory ( VK_NULL_HANDLE ),
    _imageView ( VK_NULL_HANDLE ),
    _resolution { .width = 0U, .height = 0U },
    _transfer ( VK_NULL_HANDLE ),
    _transferDeviceMemory ( VK_NULL_HANDLE ),
    _fileName ( std::move ( fileName ) )
{
    // NOTHING
}

bool Texture2D::FreeResources ( android_vulkan::Renderer &renderer )
{
    FreeTransferResources ( renderer );

    const VkDevice device = renderer.GetDevice ();

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

    if ( _image != VK_NULL_HANDLE )
    {
        vkDestroyImage ( device, _image, nullptr );
        _image = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE ( "Texture2D::_image" )
    }

    assert ( !"Texture2D::FreeResources - Implement me!" );
    return false;
}

bool Texture2D::FreeTransferResources ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _transferDeviceMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _transferDeviceMemory, nullptr );
        _transferDeviceMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "Texture2D::_transferDeviceMemory" )
    }

    if ( _transfer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transfer, nullptr );
        _transfer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "Texture2D::_transfer" )
    }

    assert ( !"Texture2D::FreeTransferResources - Implement me!" );
    return false;
}

bool Texture2D::UploadData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer )
{
    if ( _imageView != VK_NULL_HANDLE )
    {
        android_vulkan::LogWarning ( "Texture2D::UploadData - Data is uploaded already. Skipping..." );
        return true;
    }

    if ( _fileName.empty () )
    {
        android_vulkan::LogError ( "Texture2D::UploadData - Can't upload data. Filename is empty." );
        return false;
    }

    if ( !FreeResources ( renderer ) )
        return false;

    std::vector<uint8_t> pixelData;

    int width = 0;
    int height = 0;
    int channels = 0;

    if ( !LoadImage ( pixelData, _fileName, width, height, channels ) )
        return false;

    const VkFormat actualFormat = PickupFormat ( channels );

    if ( IsFormatCompatible ( _format, actualFormat ) )
        return false;

    _resolution.width = static_cast<uint32_t> ( width );
    _resolution.height = static_cast<uint32_t> ( width );

    VkImageCreateInfo imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0U;
    imageInfo.format = _format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.extent.width = _resolution.width;
    imageInfo.extent.height = _resolution.height;
    imageInfo.extent.depth = 1U;
    imageInfo.mipLevels = CountMipLevels ( _resolution );
    imageInfo.arrayLayers = 1U;
    imageInfo.queueFamilyIndexCount = 0U;
    imageInfo.pQueueFamilyIndices = nullptr;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateImage ( device, &imageInfo, nullptr, &_image ),
        "Texture2D::UploadData",
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
        "Texture2D::_imageDeviceMemory",
        "Can't allocate image memory (Texture2D::UploadData)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    result = renderer.CheckVkResult ( vkBindImageMemory ( device, _image, _imageDeviceMemory, 0U ),
        "Texture2D::UploadData",
        "Can't bind image memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0U;
    viewInfo.image = _image;
    viewInfo.format = _format;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.subresourceRange.baseMipLevel = viewInfo.subresourceRange.baseArrayLayer = 0U;
    viewInfo.subresourceRange.layerCount = 1U;
    viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    result = renderer.CheckVkResult ( vkCreateImageView ( device, &viewInfo, nullptr, &_imageView ),
        "Texture2D::UploadData",
        "Can't create image view"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_IMAGE_VIEW ( "Texture2D::_imageView" )

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0U;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0U;
    bufferInfo.pQueueFamilyIndices = nullptr;
    bufferInfo.size = static_cast<VkDeviceSize> ( pixelData.size () );

    result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transfer ),
        "Texture2D::UploadData",
        "Can't create transfer buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_BUFFER ( "Texture2D::_transfer" )

    vkGetBufferMemoryRequirements ( device, _transfer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _transferDeviceMemory,
        memoryRequirements,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        "Texture2D::_transferDeviceMemory",
        "Can't allocate transfer device memory (Texture2D::UploadData)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, _transfer, _transferDeviceMemory, 0U ),
        "Texture2D::UploadData",
        "Can't bind transfer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    void* destination = nullptr;

    result = renderer.CheckVkResult (
        vkMapMemory ( device, _transferDeviceMemory, 0U, bufferInfo.size, 0U, &destination ),
        "Texture2D::UploadData",
        "Can't map transfer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    memcpy ( destination, pixelData.data (), static_cast<size_t> ( bufferInfo.size ) );
    vkUnmapMemory ( device, _transferDeviceMemory );

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ),
        "Texture2D::UploadData",
        "Can't begin command buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkImageMemoryBarrier barrierInfo;
    barrierInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    barrierInfo.pNext = nullptr;
    barrierInfo.image = _image;
    barrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierInfo.srcAccessMask = 0U;
    barrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierInfo.srcQueueFamilyIndex = barrierInfo.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierInfo.subresourceRange.layerCount = 1U;
    barrierInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    barrierInfo.subresourceRange.baseArrayLayer = 0U;
    barrierInfo.subresourceRange.baseMipLevel = 0U;

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

    VkBufferImageCopy copyRegion;
    copyRegion.imageOffset.x = 0;
    copyRegion.imageOffset.y = 0;
    copyRegion.imageOffset.z = 0;
    copyRegion.imageExtent = imageInfo.extent;
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.layerCount = 1U;
    copyRegion.imageSubresource.baseArrayLayer = 0U;
    copyRegion.imageSubresource.mipLevel = 0U;
    copyRegion.bufferRowLength = imageInfo.extent.width;
    copyRegion.bufferImageHeight = imageInfo.extent.height;
    copyRegion.bufferOffset = 0U;

    vkCmdCopyBufferToImage ( commandBuffer, _transfer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &copyRegion );

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

    VkImageBlit blitInfo;
    blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitInfo.srcSubresource.layerCount = 1U;
    blitInfo.srcSubresource.baseArrayLayer = 0U;
    memset ( blitInfo.srcOffsets, 0, sizeof ( VkOffset3D ) );
    blitInfo.srcOffsets[ 1U ].z = 1;
    blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitInfo.dstSubresource.layerCount = 1U;
    blitInfo.dstSubresource.baseArrayLayer = 0U;
    memset ( blitInfo.dstOffsets, 0, sizeof ( VkOffset3D ) );
    blitInfo.dstOffsets[ 1U ].z = 1;

    auto commitMipLevel = [&] ( uint32_t baseMipLevel ) {
        barrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrierInfo.subresourceRange.baseMipLevel = baseMipLevel;

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
    };

    for ( uint32_t i = 1U; i < imageInfo.mipLevels; ++i )
    {
        const uint32_t previousMip = i - 1U;
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

        commitMipLevel ( previousMip );

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

    // Note last mip must must be translated to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL state.
    commitMipLevel ( imageInfo.mipLevels - 1U );

    result = renderer.CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "Texture2D::UploadData",
        "Can't end command buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;

    return renderer.CheckVkResult ( vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "Texture2D::UploadData",
        "Can't submit command"
    );
}

bool Texture2D::UploadData ( std::string& /*fileName*/,
    VkFormat /*format*/,
    android_vulkan::Renderer& /*renderer*/,
    VkCommandBuffer /*commandBuffer*/
)
{
    assert ( !"Texture2D::UploadData - Implement me!" );
    return false;
}

bool Texture2D::UploadData ( std::string&& /*fileName*/,
    VkFormat /*format*/,
    android_vulkan::Renderer& /*renderer*/,
    VkCommandBuffer /*commandBuffer*/
)
{
    assert ( !"Texture2D::UploadData - Implement me!" );
    return false;
}

bool Texture2D::UploadData ( const std::vector<uint8_t>& /*data*/,
    const VkExtent2D& /*resolution*/,
    VkFormat /*format*/,
    android_vulkan::Renderer& /*renderer*/,
    VkCommandBuffer /*commandBuffer*/
)
{
    assert ( !"Texture2D::UploadData - Implement me!" );
    return false;
}

uint32_t Texture2D::CountMipLevels ( const VkExtent2D& /*resolution*/ ) const
{
    assert ( !"Texture2D::CountMipLevels - Implement me!" );
    return 0U;
}

bool Texture2D::IsFormatCompatible ( VkFormat /*target*/, VkFormat /*candidate*/ ) const
{
    assert ( !"Texture2D::IsFormatCompatible - Implement me!" );
    return false;
}

bool Texture2D::LoadImage ( std::vector<uint8_t>& /*pixelData*/,
    const std::string& /*fileName*/,
    int& /*width*/,
    int& /*height*/,
    int& /*channels*/
) const
{
    assert ( !"Texture2D::LoadImage - Implement me!" );
    return false;
}

VkFormat Texture2D::PickupFormat ( int channels ) const
{
    switch ( channels )
    {
        case 1:
        return VK_FORMAT_R8_SRGB;

        case 2:
        return VK_FORMAT_R8G8_SRGB;

        case 3:
            android_vulkan::LogError ( "Texture2D::PickupFormat - Three channel formats are not supported!" );
        return VK_FORMAT_UNDEFINED;

        case 4:
        return VK_FORMAT_R8G8B8A8_SRGB;

        default:
            android_vulkan::LogError (
                "Texture2D::PickupFormat - Unexpected channel count: %i! Supported channel count: 1, 2 or 4."
            );
        return VK_FORMAT_UNDEFINED;
    }
}

} // namespace rotating_mesh
