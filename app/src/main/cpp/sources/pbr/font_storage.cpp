#include <pbr/font_storage.h>
#include <file.h>
#include <logger.h>
#include <vulkan_utils.h>


namespace pbr {

namespace {

constexpr float SPECIAL_GLYPH_ATLAS_LAYER = 0.0F;

} // end of anonymous namespace

bool FontStorage::StagingBuffer::Init ( android_vulkan::Renderer &renderer, uint32_t side ) noexcept
{
    auto const s = static_cast<VkDeviceSize> ( side );

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = s * s,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "pbr::FontStorage::StagingBuffer::Init",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::FontStorage::StagingBuffer::_buffer" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _memory,
        _memoryOffset,
        memoryRequirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
        "Can't allocate device memory (pbr::FontStorage::StagingBuffer::Init)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::FontStorage::StagingBuffer::_memory" )

    result = android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, _memory, _memoryOffset ),
        "pbr::FontStorage::StagingBuffer",
        "Can't bind memory"
    );

    if ( !result )
        return false;

    void* data;

    result = renderer.MapMemory ( data,
        _memory,
        _memoryOffset,
        "pbr::FontStorage::StagingBuffer::Init",
        "Can't map memory"
    );

    if ( !result )
        return false;

    _data = static_cast<uint8_t*> ( data );
    return true;
}

void FontStorage::StagingBuffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _data )
    {
        renderer.UnmapMemory ( _memory );
        _data = nullptr;
    }

    if ( _buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( renderer.GetDevice (), _buffer, nullptr );
        _buffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::FontStorage::StagingBuffer::_buffer" )
    }

    if ( _memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _memory, _memoryOffset );
    _memory = VK_NULL_HANDLE;
    _memoryOffset = 0U;
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::FontStorage::StagingBuffer::_memory" )
}

void FontStorage::StagingBuffer::Reset () noexcept
{
    _startLine =
    {
        ._height = 0U,
        ._x = 0U,
        ._y = 0U
    };

    _endLine =
    {
        ._height = 0U,
        ._x = 0U,
        ._y = 0U
    };

    _state = StagingBuffer::eState::FirstLine;
}

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::Atlas::AddLayers ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    uint32_t layers
) noexcept
{
    _oldResource = _resource;
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
            .width = _side,
            .height = _side,
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

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "pbr::FontStorage::Atlas::_image" )

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements ( device, _resource._image, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _resource._memory,
        _resource._memoryOffset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate image memory (pbr::FontStorage::Atlas::AddLayer)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::FontStorage::Atlas::_memory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindImageMemory ( device, _resource._image, _resource._memory, _resource._memoryOffset ),
        "pbr::FontStorage::Atlas::AddLayer",
        "Can't bind image memory"
    );

    if ( !result )
        return false;

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

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateImageView ( device, &viewInfo, nullptr, &_resource._view ),
        "pbr::FontStorage::Atlas::AddLayer",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE_VIEW ( "pbr::FontStorage::Atlas::_view" )

    if ( _oldResource._image != VK_NULL_HANDLE )
        Copy ( commandBuffer, layerCount );

    _layers = layerCount;
    return true;
}

void FontStorage::Atlas::Cleanup ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _oldResource._image == VK_NULL_HANDLE )
        return;

    VkDevice device = renderer.GetDevice ();
    vkDestroyImage ( device, _oldResource._image, nullptr );
    _oldResource._image = VK_NULL_HANDLE;
    AV_UNREGISTER_IMAGE ( "pbr::FontStorage::Atlas::_image" )

    if ( _oldResource._view != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _oldResource._view, nullptr );
        _oldResource._view = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "pbr::FontStorage::Atlas::_view" )
    }

    if ( _oldResource._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _oldResource._memory, _resource._memoryOffset );
    _oldResource._memory = VK_NULL_HANDLE;
    _resource._memoryOffset = 0U;
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::FontStorage::Atlas::_memory" )
}

void FontStorage::Atlas::Copy ( VkCommandBuffer commandBuffer, uint32_t newLayers ) noexcept
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
            .image = _oldResource._image,

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
                    .width = _side,
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
                .width = _side,
                .height = _side,
                .depth = 1U
            }
        };

        ++count;
    }

    vkCmdCopyImage ( commandBuffer,
        _oldResource._image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        _resource._image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        count,
        imageCopy
    );
}

void FontStorage::Atlas::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    Cleanup ( renderer );

    _oldResource = _resource;
    Cleanup ( renderer );
}

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::Init () noexcept
{
    return CheckFTResult ( FT_Init_FreeType ( &_library ), "pbr::FontStorage::Init", "Can't init FreeType" );
}

void FontStorage::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    DestroyAtlas ( renderer );

    if ( !_library )
        return;

    bool const result = CheckFTResult ( FT_Done_FreeType ( _library ),
        "pbr::FontStorage::Destroy",
        "Can't close FreeType"
    );

    if ( !result )
        return;

    _library = nullptr;
    _fontResources.clear ();
    _stringHeap.clear ();
}

std::optional<FontStorage::Font> FontStorage::GetFont ( std::string_view font, uint32_t size ) noexcept
{
    auto fontResource = GetFontResource ( font );

    if ( !fontResource )
        return std::nullopt;

    FontResource* f = *fontResource;
    auto const s = static_cast<FT_UInt> ( size );
    FT_Face face = f->_face;

    if ( !CheckFTResult ( FT_Set_Pixel_Sizes ( face, s, s ), "pbr::FontStorage::GetFont", "Can't set size" ) )
        return std::nullopt;

    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine.
    constexpr size_t magic = 0x9E3779B9U;

    static std::hash<uint32_t> const hashInteger {};
    static std::hash<void*> const hashAddress {};

    FontHash hash = 0U;
    hash ^= hashAddress ( f ) + magic + ( hash << 6U ) + ( hash >> 2U );
    hash ^= ( hashInteger ( size ) + magic + ( hash << 6U ) + ( hash >> 2U ) );

    if ( auto findResult = _fonts.find ( hash ); findResult != _fonts.end () )
        return findResult;

    auto status = _fonts.emplace ( hash,
        FontData
        {
            ._fontResource = f,
            ._fontSize = size,
            ._glyphs = {},
            ._lineHeight = static_cast<int32_t> ( static_cast<uint32_t> ( face->size->metrics.height ) >> 6U )
        }
    );

    return status.first;
}

FontStorage::GlyphInfo const& FontStorage::GetOpaqueGlyphInfo () const noexcept
{
    return _opaqueGlyph;
}

FontStorage::GlyphInfo const& FontStorage::GetTransparentGlyphInfo () const noexcept
{
    return _transparentGlyph;
}

FontStorage::GlyphInfo const& FontStorage::GetGlyphInfo ( android_vulkan::Renderer &renderer,
    Font font,
    char32_t character
) noexcept
{
    FontData& fontData = font->second;
    GlyphStorage& glyphs = fontData._glyphs;

    if ( auto const glyph = glyphs.find ( character ); glyph != glyphs.cend () )
        return glyph->second;

    return EmbedGlyph ( renderer, glyphs, fontData._fontResource->_face, fontData._fontSize, character );
}

bool FontStorage::SetMediaResolution ( android_vulkan::Renderer &renderer, VkExtent2D const &nativeViewport ) noexcept
{
    uint32_t const side = std::min ( nativeViewport.width, nativeViewport.height );

    if ( side == _atlas._side )
        return true;

    bool const result = android_vulkan::Renderer::CheckVkResult ( vkDeviceWaitIdle ( renderer.GetDevice () ),
        "pbr::FontStorage::SetMediaResolution",
        "Can't wait device idle"
    );

    if ( !result )
        return false;

    DestroyAtlas ( renderer );

    _atlas._side = side;
    _pixToUV = 1.0F / static_cast<float> ( side );

    float const h = _pixToUV * 0.5F;
    _halfPixelUV._data[ 0U ] = h;
    _halfPixelUV._data[ 1U ] = h;

    return MakeSpecialGlyphs ( renderer );
}

bool FontStorage::UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    _atlas.Cleanup ( renderer );

    if ( _activeStagingBuffer.empty () )
        return true;

    bool const isEmptyAtlas = _atlas._layers == 0U;
    size_t newLayers = 0U;

    if ( _fullStagingBuffers.empty () )
    {
        // It's needed to request one layer if atlas is empty.
        newLayers += static_cast<size_t> ( _atlas._layers == 0U );
    }
    else
    {
        // It's needed to add one more layer if full buffer in the front starts from top left corner.
        StagingBuffer const& sb = _fullStagingBuffers.front ();
        Line const& l = sb._startLine;
        newLayers += _fullStagingBuffers.size () + static_cast<size_t> ( l._x == 0U & l._y == 0U );
    }

    if ( newLayers && !_atlas.AddLayers ( renderer, commandBuffer, static_cast<uint32_t> ( newLayers ) ) )
        return false;

    uint32_t affectedLayers = _atlas._layers;

    if ( ( newLayers == 0U ) | isEmptyAtlas )
    {
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
                .layerCount = affectedLayers
            }
        };

        if ( !isEmptyAtlas )
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.subresourceRange.baseArrayLayer = _atlas._layers - 1U;
            barrier.subresourceRange.layerCount = 1U;
            affectedLayers = 1U;
        }

        vkCmdPipelineBarrier ( commandBuffer,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
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

    TransferPixels ( commandBuffer );

    VkImageMemoryBarrier const barrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = _atlas._resource._image,

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = _atlas._layers - affectedLayers,
            .layerCount = affectedLayers
        }
    };

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

int32_t FontStorage::GetKerning ( Font font, char32_t left, char32_t right ) noexcept
{
    FT_Face face = font->second._fontResource->_face;

    if ( !FT_HAS_KERNING ( face ) )
        return 0;

    FT_Vector delta {};

    bool const status = CheckFTResult (
        FT_Get_Kerning ( face,
            FT_Get_Char_Index ( face, left ),
            FT_Get_Char_Index ( face, right ),
            FT_KERNING_DEFAULT,
            &delta
        ),

        "pbr::FontStorage::GetKerning",
        "Can't resolve kerning"
    );

    int32_t const cases[] = { 0, static_cast<int32_t> ( delta.x >> 6 ) };
    return cases[ static_cast<size_t> ( status ) ];
}

void FontStorage::DestroyAtlas ( android_vulkan::Renderer &renderer ) noexcept
{
    _atlas.Destroy ( renderer );

    auto const clear = [ & ] ( auto &buffers ) noexcept {
        for ( auto& buffer : buffers )
            buffer.Destroy ( renderer );

        buffers.clear ();
    };

    clear ( _activeStagingBuffer );
    clear ( _freeStagingBuffers );
    clear ( _fullStagingBuffers );

    _fonts.clear ();
}

FontStorage::GlyphInfo const& FontStorage::EmbedGlyph ( android_vulkan::Renderer &renderer,
    GlyphStorage &glyphs,
    FT_Face face,
    uint32_t fontSize,
    char32_t character
) noexcept
{
    constexpr static GlyphInfo nullGlyph {};
    auto query = GetStagingBuffer ( renderer );

    if ( !query )
        return nullGlyph;

    StagingBuffer* stagingBuffer = query.value ();

    bool const result = CheckFTResult ( FT_Load_Char ( face, static_cast<FT_ULong> ( character ), FT_LOAD_RENDER ),
        "pbr::FontStorage::EmbedGlyph",
        "Can't set size"
    );

    if ( !result )
        return nullGlyph;

    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap const &bm = slot->bitmap;

    auto const rows = static_cast<uint32_t> ( bm.rows );
    auto const width = static_cast<size_t> ( bm.width );

    if ( ( rows == 0U ) | ( width == 0U ) )
    {
        auto const status = glyphs.emplace ( character,
            GlyphInfo
            {
                ._topLeft = _transparentGlyph._topLeft,
                ._bottomRight = _transparentGlyph._bottomRight,
                ._width = static_cast<int32_t> ( width ),
                ._height = static_cast<int32_t> ( rows ),
                ._advance = static_cast<int32_t> ( slot->advance.x ) >> 6U,
                ._offsetY = static_cast<int32_t> ( slot->metrics.height - slot->metrics.horiBearingY ) >> 6U
            }
        );

        return status.first->second;
    }

    uint32_t const side = _atlas._side;

    if ( ( rows > side ) | ( width > side ) )
    {
        android_vulkan::LogWarning ( "FontStorage::EmbedGlyph - Font size is way too big: %u", fontSize );
        return nullGlyph;
    }

    uint32_t const toRight = width - 1U;
    uint32_t const toBottom = rows - 1U;

    uint32_t lineHeight = stagingBuffer->_endLine._height;

    // Move position on one pixel right to not overwrite previously rendered glyph last column.
    uint32_t const x = stagingBuffer->_endLine._x;
    uint32_t left = x + static_cast<uint32_t> ( x > 0U );
    uint32_t top = stagingBuffer->_endLine._y;

    auto const goToNewLine = [ & ]() noexcept {
        if ( stagingBuffer->_state == StagingBuffer::eState::FirstLine )
            stagingBuffer->_startLine._height = stagingBuffer->_endLine._height;

        stagingBuffer->_state = StagingBuffer::eState::FullLinePresent;
        lineHeight = 0U;

        top += stagingBuffer->_endLine._height;
        left = 0U;
    };

    if ( left >= side )
        goToNewLine ();

    auto const completeStagingBuffer = [ & ]() noexcept -> bool {
        _fullStagingBuffers.splice ( _fullStagingBuffers.cend (),
            _activeStagingBuffer,
            _activeStagingBuffer.cbegin ()
        );

        query = GetStagingBuffer ( renderer );

        if ( !query )
            return false;

        lineHeight = 0U;
        top = 0U;
        left = 0U;
        stagingBuffer = query.value ();
        return true;
    };

    if ( top >= side && !completeStagingBuffer () )
        return nullGlyph;

    uint32_t right = left + toRight;
    uint32_t bottom = top + toBottom;

    if ( right >= side )
    {
        goToNewLine ();
        bottom = top + toBottom;
        right = toRight;
    }

    if ( bottom >= side )
    {
        if ( !completeStagingBuffer () )
            return nullGlyph;

        bottom = toBottom;
        right = toRight;
    }

    stagingBuffer->_endLine._height = std::max ( lineHeight, rows );
    stagingBuffer->_endLine._x = right;
    stagingBuffer->_endLine._y = top;
    uint8_t* data = stagingBuffer->_data + ( top * side ) + left;

    auto const* raster = static_cast<uint8_t const*> ( bm.buffer );
    auto const pitch = static_cast<ptrdiff_t> ( bm.pitch );

    // 'pitch' is positive when glyph image is stored from top to bottom line flow.
    // 'pitch' is negative when glyph image is stored from bottom to top line flow.
    // It's needed to add 'pitch' signed value to 'buffer' to go to the next line in normal top to bottom line flow.

    for ( uint32_t row = 0U; row < rows; ++row )
    {
        std::memcpy ( data, raster, static_cast<size_t> ( width ) );
        raster += pitch;
        data += static_cast<size_t> ( side );
    }

    uint32_t const cases[] = { 0U, _atlas._layers - 1U };
    uint32_t const base = cases[ static_cast<size_t> ( _atlas._layers != 0U ) ];
    auto const layer = static_cast<float> ( base + static_cast<uint32_t> ( _fullStagingBuffers.size () ) );

    auto const status = glyphs.emplace ( character,
        GlyphInfo
        {
            ._topLeft = PixToUV ( left, top, layer ),
            ._bottomRight = PixToUV ( right, bottom, layer ),
            ._width = static_cast<int32_t> ( width ),
            ._height = static_cast<int32_t> ( rows ),
            ._advance = static_cast<int32_t> ( slot->advance.x ) >> 6U,
            ._offsetY = static_cast<int32_t> ( slot->metrics.height - slot->metrics.horiBearingY ) >> 6U
        }
    );

    return status.first->second;
}

std::optional<FontStorage::FontResource*> FontStorage::GetFontResource ( std::string_view font ) noexcept
{
    if ( auto const findResult = _fontResources.find ( font ); findResult != _fontResources.cend () )
        return &findResult->second;

    android_vulkan::File fontAsset ( font );
    [[maybe_unused]] bool const result = fontAsset.LoadContent ();

    auto status = _fontResources.emplace ( std::string_view ( _stringHeap.emplace_front ( font ) ),
        FontResource
        {
            ._face = nullptr,
            ._fontAsset = std::move ( fontAsset.GetContent () )
        }
    );

    FontResource& resource = status.first->second;

    bool const init = CheckFTResult (
        FT_New_Memory_Face ( _library,
            resource._fontAsset.data (),
            static_cast<FT_Long> ( resource._fontAsset.size () ),
            0,
            &resource._face
        ),

        "pbr::FontStorage::GetFontResource",
        "Can't load face"
    );

    if ( init )
        return &resource;

    _fontResources.erase ( status.first );
    _stringHeap.pop_front ();
    return std::nullopt;
}

std::optional<FontStorage::StagingBuffer*> FontStorage::GetStagingBuffer (
    android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_activeStagingBuffer.empty () )
        return &_activeStagingBuffer.front ();

    if ( !_freeStagingBuffers.empty () )
    {
        _activeStagingBuffer.splice ( _activeStagingBuffer.cend (),
            _freeStagingBuffers,
            _freeStagingBuffers.cbegin ()
        );

        return &_activeStagingBuffer.front ();
    }

    StagingBuffer& stagingBuffer = _activeStagingBuffer.emplace_front ();

    if ( stagingBuffer.Init ( renderer, _atlas._side ) )
        return &stagingBuffer;

    return std::nullopt;
}

bool FontStorage::MakeSpecialGlyphs ( android_vulkan::Renderer &renderer ) noexcept
{
    auto query = GetStagingBuffer ( renderer );

    if ( !query )
        return false;

    GXVec3 const opaque = PixToUV ( 0U, 0U, SPECIAL_GLYPH_ATLAS_LAYER );

    _opaqueGlyph =
    {
        ._topLeft = opaque,
        ._bottomRight = opaque,
        ._width = 1,
        ._height = 1,
        ._advance = 0,
        ._offsetY = 0
    };

    GXVec3 const transparent = PixToUV ( 1U, 0U, SPECIAL_GLYPH_ATLAS_LAYER );

    _transparentGlyph =
    {
        ._topLeft = transparent,
        ._bottomRight = transparent,
        ._width = 1,
        ._height = 1,
        ._advance = 0,
        ._offsetY = 0
    };

    StagingBuffer &stagingBuffer = *query.value ();
    stagingBuffer._data[ 0U ] = std::numeric_limits<uint8_t>::max ();
    stagingBuffer._data[ 1U ] = 0U;

    stagingBuffer._endLine._x = 1U;
    stagingBuffer._endLine._height = 1U;

    return true;
}

GXVec3 FontStorage::PixToUV ( uint32_t x, uint32_t y, float layer ) const noexcept
{
    GXVec2 a {};
    a.Sum ( _halfPixelUV, _pixToUV, GXVec2 ( static_cast<float> ( x ), static_cast<float> ( y ) ) );
    return GXVec3 ( a._data[ 0U ], a._data[ 1U ], layer );
}

void FontStorage::TransferPixels ( VkCommandBuffer commandBuffer ) noexcept
{
    VkBufferImageCopy bufferImageCopy[ 3U ];
    uint32_t targetLayer = _atlas._layers - 1U;

    auto const offset = [ side = static_cast<VkDeviceSize> ( _atlas._side ) ] ( uint32_t x,
        uint32_t y
    ) noexcept -> VkDeviceSize
    {
        return static_cast<VkDeviceSize> ( y ) * side + static_cast<VkDeviceSize> ( x );
    };

    auto const transferComplex = [ & ] ( StagingBuffer const &b ) noexcept {
        if ( b._startLine._y == b._endLine._y )
        {
            // Single area case.
            bufferImageCopy[ 0U ] =
            {
                .bufferOffset = offset ( b._startLine._x, b._startLine._y ),
                .bufferRowLength = _atlas._side,
                .bufferImageHeight = b._endLine._height,

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
                    .width = b._endLine._x - b._startLine._x + 1U,
                    .height = b._endLine._height,
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
            .bufferRowLength = _atlas._side,
            .bufferImageHeight = b._startLine._height,

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
                .width = _atlas._side - b._startLine._x,
                .height = b._startLine._height,
                .depth = 1U,
            }
        };

        uint32_t const nextY = b._startLine._y + b._startLine._height;
        size_t idx = 1U;

        if ( nextY != b._endLine._y )
        {
            // Middle area.
            bufferImageCopy[ 1U ] =
            {
                .bufferOffset = offset ( 0U, nextY ),
                .bufferRowLength = _atlas._side,
                .bufferImageHeight = b._endLine._y - nextY,

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
                    .width = _atlas._side,
                    .height = b._endLine._y - nextY,
                    .depth = 1U,
                }
            };

            idx = 2U;
        }

        // End line area.
        bufferImageCopy[ idx ] =
        {
            .bufferOffset = offset ( 0U, b._endLine._y ),
            .bufferRowLength = _atlas._side,
            .bufferImageHeight = b._endLine._height,

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
                .height = b._endLine._height,
                .depth = 1U,
            }
        };

        vkCmdCopyBufferToImage ( commandBuffer,
            b._buffer,
            _atlas._resource._image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            idx + 1U,
            bufferImageCopy
        );
    };

    if ( !_fullStagingBuffers.empty () )
    {
        // First full buffer is special is could have one, two or three areas.
        StagingBuffer &b = _fullStagingBuffers.front ();
        transferComplex ( b );
        b.Reset ();

        _freeStagingBuffers.splice ( _freeStagingBuffers.cend (),
            _fullStagingBuffers,
            _fullStagingBuffers.cbegin ()
        );

        ++targetLayer;
    }

    while ( !_fullStagingBuffers.empty () )
    {
        // Rest full buffers contain only one area: entire atlas layer.
        StagingBuffer &b = _fullStagingBuffers.front ();

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
                .width = _atlas._side,
                .height = _atlas._side,
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

        _freeStagingBuffers.splice ( _freeStagingBuffers.cend (),
            _fullStagingBuffers,
            _fullStagingBuffers.cbegin ()
        );

        ++targetLayer;
    }

    StagingBuffer &b = _activeStagingBuffer.front ();
    transferComplex ( b );

    _freeStagingBuffers.splice ( _freeStagingBuffers.cend (),
        _activeStagingBuffer,
        _activeStagingBuffer.cbegin ()
    );

    _atlas._line = b._endLine;

    b._startLine._x = b._endLine._x + 1U;
    b._startLine._y = b._endLine._y;
    b._startLine._height = b._endLine._height;
    b._state = StagingBuffer::eState::FirstLine;

    if ( b._startLine._x >= _atlas._side )
    {
        b._startLine._height = 0U;
        b._startLine._x = 0U;
        b._startLine._y += b._endLine._height;
    }

    if ( b._startLine._y < _atlas._side )
    {
        b._endLine = b._startLine;
        return;
    }

    b.Reset ();
}

bool FontStorage::CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept
{
    if ( result == FT_Err_Ok )
        return true;

    android_vulkan::LogError ( "%s - %s. Error: %s.", from, message, FT_Error_String ( result ) );
    return false;
}

} // namespace pbr
