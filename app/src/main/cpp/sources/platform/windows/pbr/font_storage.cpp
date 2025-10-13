#include <precompiled_headers.hpp>
#include <platform/windows/pbr/font_storage.hpp>


namespace pbr {

void FontStagingBuffer::AddGlyph ( GlyphStorage<GlyphInfo> &glyphs, char32_t character ) noexcept
{
    _bindings.push_back (
        {
            ._glyphs = &glyphs,
            ._character = character
        }
    );
}

void FontStagingBuffer::BindGlyphs ( uint16_t heapResource ) noexcept
{
    for ( auto const &bind : _bindings )
        bind._glyphs->at ( bind._character )._pageID = heapResource;

    _bindings.clear ();
}

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::Atlas::AddPages ( android_vulkan::Renderer &renderer,
    ResourceHeap &resourceHeap,
    uint32_t pages
) noexcept
{
    constexpr VkImageCreateInfo imageInfo
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
        .arrayLayers = 1U,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_DST_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkImageViewCreateInfo viewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = VK_NULL_HANDLE,
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
            .layerCount = 1U
        }
    };

    uint32_t i = static_cast<uint32_t> ( _pages.size () );
    uint32_t const limit = pages + i;
    _pages.resize ( static_cast<size_t> ( limit  ) );
    ImageResource* p = _pages.data ();
    VkDevice device = renderer.GetDevice ();

    for ( ; i < limit; ++i )
    {
        ImageResource &resource = p[ i ];

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateImage ( device, &imageInfo, nullptr, &resource._image ),
            "pbr::FontStorage::Atlas::AddLayer",
            "Can't create image"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, resource._image, VK_OBJECT_TYPE_IMAGE, "Font atlas page #%u", i )
        viewInfo.image = resource._image;

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements ( device, resource._image, &memoryRequirements );

        result =
            renderer.TryAllocateMemory ( resource._memory,
                resource._memoryOffset,
                memoryRequirements,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                "Can't allocate image memory (pbr::FontStorage::Atlas::AddLayer)"
            ) &&


            android_vulkan::Renderer::CheckVkResult (
                vkBindImageMemory ( device, resource._image, resource._memory, resource._memoryOffset ),
                "pbr::FontStorage::Atlas::AddLayer",
                "Can't bind image memory"
            ) &&

            android_vulkan::Renderer::CheckVkResult (
                vkCreateImageView ( device, &viewInfo, nullptr, &resource._view ),
                "pbr::FontStorage::Atlas::AddLayer",
                "Can't create image view"
            );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device,
            resource._view,
            VK_OBJECT_TYPE_IMAGE_VIEW,
            "Font storage atlas page #%u",
            i
        )

        if ( auto const idx = resourceHeap.RegisterUISampledImage ( device, resource._view ); idx ) [[unlikely]]
        {
            resource._heapResource = static_cast<uint16_t> ( *idx );
            continue;
        }

        return false;
    }

    return true;
}

void FontStorage::Atlas::Destroy ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    for ( ImageResource &page : _pages )
    {
        if ( page._heapResource != ResourceHeap::INVALID_UI_IMAGE ) [[likely]]
            resourceHeap.UnregisterResource ( static_cast<uint32_t> ( page._heapResource ) );

        if ( page._view != VK_NULL_HANDLE ) [[likely]]
            vkDestroyImageView ( device, page._view, nullptr );

        if ( page._image != VK_NULL_HANDLE ) [[likely]]
            vkDestroyImage ( device, page._image, nullptr );

        if ( page._memory != VK_NULL_HANDLE ) [[likely]]
        {
            renderer.FreeMemory ( page._memory, page._memoryOffset );
        }
    }

    _pages.clear ();
    _pages.shrink_to_fit ();
}

//----------------------------------------------------------------------------------------------------------------------

FontStorage::FontStorage ( ResourceHeap &resourceHeap ) noexcept:
    _resourceHeap ( resourceHeap )
{
    // NOTHING
}

bool FontStorage::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    return InitBase () && MakeTransparentGlyph ( renderer );
}

void FontStorage::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _atlas.Destroy ( renderer, _resourceHeap );
    DestroyBase ( renderer );
}

bool FontStorage::UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    if ( _activeStagingBuffer.empty () ) [[likely]]
        return true;

    auto const wasPages = static_cast<uint16_t> ( _atlas._pages.size () );
    auto const emptyAtlasCorrection = static_cast<uint16_t> ( wasPages == 0U );

    // Considering scenario: no full staging buffers.
    // It's needed to request one layer if atlas is empty.
    uint16_t newLayers = emptyAtlasCorrection;
    uint16_t neededBarriers = 1U;
    uint16_t pageIndexCases[] = { wasPages - 1U, 0U };

    if ( size_t const fullBuffers = _fullStagingBuffers.size (); fullBuffers > 0U ) [[unlikely]]
    {
        // It's needed to add one more layer if full buffer in the front starts from top left corner.
        FontAtlasLine const &line = _fullStagingBuffers.front ()._startLine;
        auto const partialCorrection = static_cast<uint16_t> ( static_cast<uint16_t> ( line._x + line._y ) > 0U );
        neededBarriers = static_cast<uint16_t> ( 1U + fullBuffers );
        newLayers = static_cast<uint16_t> ( neededBarriers - partialCorrection );
        pageIndexCases[ 0U ] = static_cast<uint16_t> ( wasPages - partialCorrection );
    }

    if ( newLayers > 0U && !_atlas.AddPages ( renderer, _resourceHeap, static_cast<uint32_t> ( newLayers ) ) )
    {
        [[unlikely]]
        return false;
    }

    ImageResource const* pages = _atlas._pages.data () + pageIndexCases[ emptyAtlasCorrection ];

    if ( _transparentGlyph._pageID == ResourceHeap::INVALID_UI_IMAGE ) [[unlikely]]
        _transparentGlyph._pageID = pages->_heapResource;

    uint16_t i = 0U;

    for ( auto &fullStagingBuffer : _fullStagingBuffers )
        fullStagingBuffer.BindGlyphs ( pages[ i++ ]._heapResource );

    _activeStagingBuffer.front ().BindGlyphs ( pages[ i ]._heapResource );

    constexpr VkImageMemoryBarrier2 barrierTemplate
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = VK_NULL_HANDLE,

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    VkImageMemoryBarrier2* barriers = _barriers.data ();
    _dependencies.imageMemoryBarrierCount = static_cast<uint32_t> ( neededBarriers );

    if ( auto const count = static_cast<uint16_t> ( _barriers.size () ); count < neededBarriers ) [[unlikely]]
    {
        _barriers.reserve ( static_cast<size_t> ( neededBarriers ) );

        for ( i = count; i < neededBarriers; ++i )
            _barriers.push_back ( barrierTemplate );

        barriers = _barriers.data ();
        _dependencies.pImageMemoryBarriers = barriers;
    }

    i = 0U;

    if ( ( newLayers != neededBarriers ) | emptyAtlasCorrection ) [[likely]]
    {
        constexpr VkPipelineStageFlags2 const stageCases[] =
        {
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
        };

        constexpr VkAccessFlags2 const accessCases[] = { VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_NONE };

        constexpr VkImageLayout const layoutCases[] =
        {
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_UNDEFINED
        };

        VkImageMemoryBarrier2 &b = barriers[ i ];
        b.srcStageMask = stageCases[ emptyAtlasCorrection ],
        b.srcAccessMask = accessCases[ emptyAtlasCorrection ];
        b.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        b.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        b.oldLayout = layoutCases[ emptyAtlasCorrection ];
        b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        b.image = pages[ i++ ]._image;
    }

    for ( ; i < neededBarriers; ++i )
    {
        VkImageMemoryBarrier2 &b = barriers[ i ];
        b.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
        b.srcAccessMask = VK_ACCESS_2_NONE;
        b.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        b.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        b.image = pages[ i ]._image;
    }

    vkCmdPipelineBarrier2 ( commandBuffer, &_dependencies );
    TransferPixels ( commandBuffer, pages );

    for ( i = 0U; i < neededBarriers; ++i )
    {
        VkImageMemoryBarrier2 &b = barriers[ i ];
        b.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        b.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        b.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        b.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        b.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    vkCmdPipelineBarrier2 ( commandBuffer, &_dependencies );
    return true;
}

GlyphInfo const &FontStorage::InsertGlyph ( GlyphStorage<GlyphInfo> &glyphs,
    FontStagingBufferBase &stagingBuffer,
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
    // NOLINTNEXTLINE - downcast
    static_cast<FontStagingBuffer &> ( stagingBuffer ).AddGlyph ( glyphs, character );

    auto const status = glyphs.insert (
        std::make_pair ( character,

            GlyphInfo
            {
                ._topLeft = topLeft,
                ._bottomRight = bottomRight,
                ._pageID = ResourceHeap::INVALID_UI_IMAGE,
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
        ._pageID = ResourceHeap::INVALID_UI_IMAGE,
        ._width = 1,
        ._height = 1,
        ._advance = 0,
        ._offsetY = 0
    };

    FontStagingBuffer &stagingBuffer = *query.value ();
    stagingBuffer._data[ 0U ] = 0U;

    stagingBuffer._endLine =
    {
        ._height = 1U,
        ._x = 0U,
        ._y = 0U
    };

    return true;
}

void FontStorage::TransferPixels ( VkCommandBuffer commandBuffer, ImageResource const* pages ) noexcept
{
    constexpr auto offset = [] ( uint16_t x, uint16_t y ) noexcept -> VkDeviceSize {
        return static_cast<VkDeviceSize> ( x ) +
            static_cast<VkDeviceSize> ( y ) * static_cast<VkDeviceSize> ( FONT_ATLAS_RESOLUTION );
    };

    auto const transferComplex = [ & ] ( FontStagingBuffer const &b ) noexcept {
        VkBufferImageCopy &imageCopy = _imageCopy;

        if ( b._startLine._y == b._endLine._y )
        {
            // Single area case.
            imageCopy.bufferOffset = offset ( b._startLine._x, b._startLine._y );
            imageCopy.bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION );
            imageCopy.bufferImageHeight = static_cast<uint32_t> ( b._endLine._height );
            imageCopy.imageOffset.x = static_cast<int32_t> ( b._startLine._x );
            imageCopy.imageOffset.y = static_cast<int32_t> ( b._endLine._y );
            imageCopy.imageExtent.width = static_cast<uint32_t> ( b._endLine._x - b._startLine._x + 1U );
            imageCopy.imageExtent.height = static_cast<uint32_t> ( b._endLine._height );

            vkCmdCopyBufferToImage ( commandBuffer,
                b._buffer,
                ( pages++ )->_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1U,
                &imageCopy
            );

            return;
        }

        // Two or three areas. Definitely has distinct first line and end line areas,
        // Middle area is questionable.

        // First line area.
        imageCopy.bufferOffset = offset ( b._startLine._x, b._startLine._y );
        imageCopy.bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION );
        imageCopy.bufferImageHeight = static_cast<uint32_t> ( b._startLine._height );
        imageCopy.imageOffset.x = static_cast<int32_t> ( b._startLine._x );
        imageCopy.imageOffset.y = static_cast<int32_t> ( b._startLine._y );
        imageCopy.imageExtent.width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION - b._startLine._x );
        imageCopy.imageExtent.height = b._startLine._height;

        vkCmdCopyBufferToImage ( commandBuffer,
            b._buffer,
            ( pages++ )->_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            &imageCopy
        );

        auto const nextY = static_cast<uint16_t> ( b._startLine._y + b._startLine._height );

        if ( nextY != b._endLine._y )
        {
            // Middle area.
            imageCopy.bufferOffset = offset ( 0U, nextY );
            imageCopy.bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION );
            imageCopy.bufferImageHeight = static_cast<uint32_t> ( b._endLine._y - nextY );
            imageCopy.imageOffset.x = 0;
            imageCopy.imageOffset.y = static_cast<int32_t> ( nextY );
            imageCopy.imageExtent.width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION );
            imageCopy.imageExtent.height = static_cast<uint32_t> ( b._endLine._y - nextY );

            vkCmdCopyBufferToImage ( commandBuffer,
                b._buffer,
                ( pages++ )->_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1U,
                &imageCopy
            );
        }

        // End line area.
        imageCopy.bufferOffset = offset ( 0U, b._endLine._y );
        imageCopy.bufferRowLength = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION );
        imageCopy.bufferImageHeight = static_cast<uint32_t> ( b._endLine._height );
        imageCopy.imageOffset.x = 0;
        imageCopy.imageOffset.y = static_cast<int32_t> ( b._endLine._y );
        imageCopy.imageExtent.width = b._endLine._x + 1U;
        imageCopy.imageExtent.height = static_cast<uint32_t> ( b._endLine._height );

        vkCmdCopyBufferToImage ( commandBuffer,
            b._buffer,
            ( pages++ )->_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            &imageCopy
        );
    };

    auto begin = _fullStagingBuffers.begin ();
    auto const end = _fullStagingBuffers.cend ();

    if ( begin != end )
    {
        // First full buffer is special: it could have one, two or three areas.
        FontStagingBuffer &b = *begin;
        transferComplex ( b );
        b.Reset ();

        ++begin;
    }

    for ( ; begin != end; ++begin )
    {
        // Rest full buffers contain only one area: entire atlas layer.
        FontStagingBuffer &b = *begin;
        VkBufferImageCopy &imageCopy = _imageCopy;

        imageCopy.bufferOffset = 0U;
        imageCopy.bufferRowLength = 0U;
        imageCopy.bufferImageHeight = 0U;
        imageCopy.imageOffset.x = 0;
        imageCopy.imageOffset.y = 0;
        imageCopy.imageExtent.width = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION );
        imageCopy.imageExtent.height = static_cast<uint32_t> ( FONT_ATLAS_RESOLUTION );

        vkCmdCopyBufferToImage ( commandBuffer,
            b._buffer,
            ( pages++ )->_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1U,
            &imageCopy
        );

        b.Reset ();
    }

    _freeStagingBuffers.splice ( _freeStagingBuffers.cend (), _fullStagingBuffers );

    begin = _activeStagingBuffer.begin ();
    FontStagingBuffer &b = *begin;
    transferComplex ( b );
    _freeStagingBuffers.splice ( _freeStagingBuffers.cend (), _activeStagingBuffer, begin );

    _atlas._line = b._endLine;

    b._startLine =
    {
        ._height = b._endLine._height,
        ._x = static_cast<uint16_t> ( b._endLine._x + 1U ),
        ._y = b._endLine._y
    };

    b._state = FontStagingBuffer::eState::FirstLine;

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
