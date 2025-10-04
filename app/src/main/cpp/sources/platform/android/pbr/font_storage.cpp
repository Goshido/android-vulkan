#include <precompiled_headers.hpp>
#include <platform/android/pbr/font_storage.hpp>


namespace pbr {

namespace {

constexpr uint8_t TRANSPARENT_GLYPH_ATLAS_LAYER = 0U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::Atlas::AddLayers ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    size_t commandBufferIndex,
    uint32_t layers
) noexcept
{
    ImageResource &oldResource = _dyingResources[ commandBufferIndex ];
    oldResource = _resource;
    uint32_t const layerCount = _layers + layers;

    VkImageCreateInfo const imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8_UNORM,

        .extent
        {
            .width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
            .height = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
            .depth = 1U
        },

        .mipLevels = 1U,
        .arrayLayers = layerCount,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,

        .usage = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_SRC_BIT ) |
            AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_DST_BIT ),

        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateImage ( device, &imageInfo, nullptr, &_resource._image ),
        "pbr::FontStorage::Atlas::AddLayer",
        "Can't create image"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _resource._image, VK_OBJECT_TYPE_IMAGE, "Font atlas" )

    VkImageViewCreateInfo const viewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = _resource._image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        .format = imageInfo.format,

        .components
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = layerCount
        }
    };

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _resource._image, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _resource._memory,
            _resource._memoryOffset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Can't allocate image memory (pbr::FontStorage::Atlas::AddLayer)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindImageMemory ( device, _resource._image, _resource._memory, _resource._memoryOffset ),
            "pbr::FontStorage::Atlas::AddLayer",
            "Can't bind image memory"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkCreateImageView ( device, &viewInfo, nullptr, &_resource._view ),
            "pbr::FontStorage::Atlas::AddLayer",
            "Can't create image view"
        );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _resource._view, VK_OBJECT_TYPE_IMAGE_VIEW, "Font storage atlas" )

    if ( oldResource._image != VK_NULL_HANDLE )
        Copy ( commandBuffer, oldResource, layerCount );

    _layers = layerCount;
    return true;
}

void FontStorage::Atlas::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    for ( ImageResource &imageResource : _dyingResources )
        FreeImageResource ( renderer, imageResource );

    FreeImageResource ( renderer, _resource );
    _layers = 0U;
}

void FontStorage::Atlas::Cleanup ( android_vulkan::Renderer &renderer, size_t commandBufferIndex ) noexcept
{
    FreeImageResource ( renderer, _dyingResources[ commandBufferIndex ] );
}

void FontStorage::Atlas::FreeImageResource ( android_vulkan::Renderer &renderer, ImageResource &resource ) noexcept
{
    if ( resource._image == VK_NULL_HANDLE ) [[unlikely]]
        return;

    VkDevice device = renderer.GetDevice ();
    vkDestroyImage ( device, resource._image, nullptr );
    resource._image = VK_NULL_HANDLE;

    if ( resource._view != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyImageView ( device, resource._view, nullptr );
        resource._view = VK_NULL_HANDLE;
    }

    if ( resource._memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    renderer.FreeMemory ( resource._memory, resource._memoryOffset );
    resource._memory = VK_NULL_HANDLE;
    resource._memoryOffset = 0U;
}

void FontStorage::Atlas::Copy ( VkCommandBuffer commandBuffer, ImageResource &oldResource, uint32_t newLayers ) noexcept
{
    VkImageMemoryBarrier const barriers[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = oldResource._image,

            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0U,
                .levelCount = 1U,
                .baseArrayLayer = 0U,
                .layerCount = _layers
            }
        },
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = _resource._image,

            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0U,
                .levelCount = 1U,
                .baseArrayLayer = 0U,
                .layerCount = newLayers
            }
        }
    };

    vkCmdPipelineBarrier ( commandBuffer,
        AV_VK_FLAG ( VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ) | AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        0U,
        nullptr,
        static_cast<uint32_t> ( std::size ( barriers ) ),
        barriers
    );

    uint32_t fullLayers = _layers;
    VkImageCopy imageCopy[ 3U ];
    uint32_t count = 0U;

    if ( _line._height != 0U )
    {
        --fullLayers;

        imageCopy[ 0U ] =
        {
            .srcSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = fullLayers,
                .layerCount = 1U
            },

            .srcOffset
            {
                .x = 0,
                .y = static_cast<int32_t> ( _line._y ),
                .z = 0
            },

            .dstSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = fullLayers,
                .layerCount = 1U
            },

            .dstOffset
            {
                .x = 0,
                .y = static_cast<int32_t> ( _line._y ),
                .z = 0
            },

            .extent
            {
                .width = _line._x + 1U,
                .height = _line._height,
                .depth = 1U
            }
        };

        count = 1U;

        if ( _line._y > 0U )
        {
            imageCopy[ 1U ] =
            {
                .srcSubresource
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0U,
                    .baseArrayLayer = fullLayers,
                    .layerCount = 1U
                },

                .srcOffset
                {
                    .x = 0,
                    .y = 0,
                    .z = 0
                },

                .dstSubresource
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0U,
                    .baseArrayLayer = fullLayers,
                    .layerCount = 1U
                },

                .dstOffset
                {
                    .x = 0,
                    .y = 0,
                    .z = 0
                },

                .extent
                {
                    .width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                    .height = _line._y + 1U,
                    .depth = 1U
                }
            };

            count = 2U;
        }
    }

    if ( fullLayers > 0U )
    {
        imageCopy[ count ] =
        {
            .srcSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = 0U,
                .layerCount = fullLayers
            },

            .srcOffset
            {
                .x = 0,
                .y = 0,
                .z = 0
            },

            .dstSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = 0U,
                .layerCount = fullLayers
            },

            .dstOffset
            {
                .x = 0,
                .y = 0,
                .z = 0
            },

            .extent
            {
                .width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                .height = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                .depth = 1U
            }
        };

        ++count;
    }

    vkCmdCopyImage ( commandBuffer,
        oldResource._image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        _resource._image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        count,
        imageCopy
    );
}

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    return InitBase () && MakeTransparentGlyph ( renderer );
}

void FontStorage::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _atlas.Destroy ( renderer );
    DestroyBase ( renderer );
}

VkImageView FontStorage::GetAtlasImageView () const noexcept
{
    return _atlas._resource._view;
}

bool FontStorage::UploadGPUData ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    size_t commandBufferIndex
) noexcept
{
    _atlas.Cleanup ( renderer, commandBufferIndex );

    if ( _activeStagingBuffer.empty () ) [[likely]]
        return true;

    uint32_t const wasLayers = _atlas._layers;
    bool const isEmptyAtlas = wasLayers == 0U;
    auto const emptyAtlasCorrection = static_cast<size_t> ( isEmptyAtlas );
    size_t newLayers;

    if ( _fullStagingBuffers.empty () )
    {
        // It's needed to request one layer if atlas is empty.
        newLayers = emptyAtlasCorrection;
    }
    else
    {
        // It's needed to add one more layer if full buffer in the front starts from top left corner.
        FontAtlasLine const &line = _fullStagingBuffers.front ()._startLine;
        auto const partialCorrection = static_cast<size_t> ( static_cast<uint16_t> ( line._x + line._y ) > 0U );
        newLayers = 1U + _fullStagingBuffers.size () - partialCorrection;
    }

    if ( newLayers ) [[unlikely]]
    {
        if ( !_atlas.AddLayers ( renderer, commandBuffer, commandBufferIndex, static_cast<uint32_t> ( newLayers ) ) )
        {
            [[unlikely]]
            return false;
        }
    }

    VkImageMemoryBarrier barrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_NONE,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = _atlas._resource._image,

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = _atlas._layers
        }
    };

    if ( ( newLayers == 0U ) | isEmptyAtlas )
    {
        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        if ( !isEmptyAtlas )
        {
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.subresourceRange.baseArrayLayer = _atlas._layers - 1U;
            barrier.subresourceRange.layerCount = 1U;
        }

        vkCmdPipelineBarrier ( commandBuffer,
            srcStage,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_DEPENDENCY_BY_REGION_BIT,
            0U,
            nullptr,
            0U,
            nullptr,
            1U,
            &barrier
        );
    }

    TransferPixels ( commandBuffer, wasLayers - 1U + static_cast<uint32_t> ( emptyAtlasCorrection ) );

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,

    barrier.subresourceRange.baseArrayLayer =
        _atlas._layers - static_cast<size_t> ( barrier.subresourceRange.layerCount );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrier
    );

    return true;
}

GlyphInfo const &FontStorage::InsertGlyph ( GlyphStorage<GlyphInfo> &glyphs,
    FontStagingBufferBase &/*stagingBuffer*/,
    char32_t character,
    int32_t offsetX,
    int32_t offsetY,
    int32_t advance,
    int32_t width,
    int32_t height,
    android_vulkan::Half2 topLeft,
    android_vulkan::Half2 bottomRight
) noexcept
{
    uint32_t const cases[] = { 0U, _atlas._layers - 1U };
    uint32_t const base = cases[ static_cast<size_t> ( _atlas._layers != 0U ) ];

    auto const status = glyphs.insert (
        std::make_pair ( character,

            GlyphInfo
            {
                ._topLeft = topLeft,
                ._bottomRight = bottomRight,
                ._pageID = static_cast<uint8_t> ( base + static_cast<uint32_t> ( _fullStagingBuffers.size () ) ),
                ._width = width,
                ._height = height,
                ._advance = advance,
                ._offsetX = offsetX,
                ._offsetY = offsetY
            }
        )
    );

    return status.first->second;
}

bool FontStorage::MakeTransparentGlyph ( android_vulkan::Renderer &renderer ) noexcept
{
    auto query = GetStagingBuffer ( renderer );

    if ( !query ) [[unlikely]]
        return false;

    android_vulkan::Half2 const transparent = PixToUV ( 0U, 0U );

    _transparentGlyph =
    {
        ._topLeft = transparent,
        ._bottomRight = transparent,
        ._pageID = TRANSPARENT_GLYPH_ATLAS_LAYER,
        ._width = 1,
        ._height = 1,
        ._advance = 0,
        ._offsetY = 0
    };

    FontStagingBufferBase &stagingBuffer = *query.value ();
    stagingBuffer._data[ 0U ] = 0U;

    stagingBuffer._endLine =
    {
        ._height = 1U,
        ._x = 0U,
        ._y = 0U
    };

    return true;
}

void FontStorage::TransferPixels ( VkCommandBuffer commandBuffer, uint32_t targetLayer ) noexcept
{
    VkBufferImageCopy bufferImageCopy[ 3U ];

    constexpr auto offset = [] ( uint16_t x, uint16_t y ) noexcept -> VkDeviceSize {
        return static_cast<VkDeviceSize> ( x ) +
            static_cast<VkDeviceSize> ( y ) * static_cast<VkDeviceSize> ( FONT_ATLAS_RESOLUTION );
    };

    auto const transferComplex = [ & ] ( FontStagingBufferBase const &b ) noexcept {
        if ( b._startLine._y == b._endLine._y )
        {
            // Single area case.
            bufferImageCopy[ 0U ] =
            {
                .bufferOffset = offset ( b._startLine._x, b._startLine._y ),
                .bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                .bufferImageHeight = static_cast<uint32_t> ( b._endLine._height ),

                .imageSubresource
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0U,
                    .baseArrayLayer = targetLayer,
                    .layerCount = 1U
                },

                .imageOffset
                {
                    .x = static_cast<int32_t> ( b._startLine._x ),
                    .y = static_cast<int32_t> ( b._endLine._y ),
                    .z = 0
                },

                .imageExtent
                {
                    .width = static_cast<uint32_t> ( b._endLine._x - b._startLine._x + 1U ),
                    .height = static_cast<uint32_t> (  b._endLine._height ),
                    .depth = 1U,
                }
            };

            vkCmdCopyBufferToImage ( commandBuffer,
                b._buffer,
                _atlas._resource._image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1U,
                bufferImageCopy
            );

            return;
        }

        // Two or three areas. Definitely has distinct first line and end line areas,
        // Middle area is questionable.

        // First line area.
        bufferImageCopy[ 0U ] =
        {
            .bufferOffset = offset ( b._startLine._x, b._startLine._y ),
            .bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
            .bufferImageHeight = static_cast<uint32_t> ( b._startLine._height ),

            .imageSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = targetLayer,
                .layerCount = 1U
            },

            .imageOffset
            {
                .x = static_cast<int32_t> ( b._startLine._x ),
                .y = static_cast<int32_t> ( b._startLine._y ),
                .z = 0
            },

            .imageExtent
            {
                .width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION - b._startLine._x ),
                .height = b._startLine._height,
                .depth = 1U,
            }
        };

        auto const nextY = static_cast<uint16_t> ( b._startLine._y + b._startLine._height );
        size_t idx = 1U;

        if ( nextY != b._endLine._y )
        {
            // Middle area.
            bufferImageCopy[ 1U ] =
            {
                .bufferOffset = offset ( 0U, nextY ),
                .bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                .bufferImageHeight = static_cast<uint32_t> ( b._endLine._y - nextY ),

                .imageSubresource
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0U,
                    .baseArrayLayer = targetLayer,
                    .layerCount = 1U
                },

                .imageOffset
                {
                    .x = 0,
                    .y = static_cast<int32_t> ( nextY ),
                    .z = 0
                },

                .imageExtent
                {
                    .width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                    .height = static_cast<uint32_t> ( b._endLine._y - nextY ),
                    .depth = 1U,
                }
            };

            idx = 2U;
        }

        // End line area.
        bufferImageCopy[ idx ] =
        {
            .bufferOffset = offset ( 0U, b._endLine._y ),
            .bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
            .bufferImageHeight = static_cast<uint32_t> ( b._endLine._height ),

            .imageSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = targetLayer,
                .layerCount = 1U
            },

            .imageOffset
            {
                .x = 0,
                .y = static_cast<int32_t> ( b._endLine._y ),
                .z = 0
            },

            .imageExtent
            {
                .width = b._endLine._x + 1U,
                .height = static_cast<uint32_t> ( b._endLine._height ),
                .depth = 1U,
            }
        };

        vkCmdCopyBufferToImage ( commandBuffer,
            b._buffer,
            _atlas._resource._image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t> ( idx + 1U ),
            bufferImageCopy
        );
    };

    auto begin = _fullStagingBuffers.begin ();
    auto const end = _fullStagingBuffers.cend ();

    if ( begin != end )
    {
        // First full buffer is special: it could have one, two or three areas.
        FontStagingBufferBase &b = *begin;
        transferComplex ( b );
        b.Reset ();

        ++targetLayer;
        ++begin;
    }

    for ( ; begin != end; ++begin )
    {
        // Rest full buffers contain only one area: entire atlas layer.
        FontStagingBufferBase &b = *begin;

        bufferImageCopy[ 0U ] =
        {
            .bufferOffset = 0U,
            .bufferRowLength = 0U,
            .bufferImageHeight = 0U,

            .imageSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = targetLayer,
                .layerCount = 1U
            },

            .imageOffset
            {
                .x = 0,
                .y = 0,
                .z = 0
            },

            .imageExtent
            {
                .width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                .height = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION ),
                .depth = 1U,
            }
        };

        vkCmdCopyBufferToImage ( commandBuffer,
            b._buffer,
            _atlas._resource._image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            bufferImageCopy
        );

        b.Reset ();
        ++targetLayer;
    }

    _freeStagingBuffers.splice ( _freeStagingBuffers.cend (), _fullStagingBuffers );

    begin = _activeStagingBuffer.begin ();
    FontStagingBufferBase &b = *begin;
    transferComplex ( b );
    _freeStagingBuffers.splice ( _freeStagingBuffers.cend (), _activeStagingBuffer, begin );

    _atlas._line = b._endLine;

    b._startLine =
    {
        ._height = b._endLine._height,
        ._x = static_cast<uint16_t> ( b._endLine._x + 1U ),
        ._y = b._endLine._y
    };

    b._state = FontStagingBufferBase::eState::FirstLine;

    if ( b._startLine._x >= FONT_ATLAS_RESOLUTION ) [[unlikely]]
    {
        b._startLine =
        {
            ._height = 0U,
            ._x = 0U,
            ._y = static_cast<uint16_t> ( b._startLine._y + b._endLine._height )
        };
    }

    if ( b._startLine._y >= FONT_ATLAS_RESOLUTION ) [[unlikely]]
    {
        b.Reset ();
        return;
    }

    b._endLine = b._startLine;
}

} // namespace pbr
