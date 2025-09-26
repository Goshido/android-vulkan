#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <platform/windows/pbr/font_storage.hpp>


namespace pbr {

namespace {

constexpr uint16_t FONT_ATLAS_RESOLUTION = 1024U;

} // end of anonymous namespace

FontStorage::FontLock::FontLock ( Font font, std::shared_lock<std::shared_mutex> &&lock ) noexcept:
    _font ( font ),
    _lock ( std::move ( lock ) )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::StagingBuffer::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr auto s = static_cast<VkDeviceSize> ( FONT_ATLAS_RESOLUTION );

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

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "Font storage staging" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _memory,
        _memoryOffset,
        memoryRequirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
        "Can't allocate device memory (pbr::FontStorage::StagingBuffer::Init)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, _memory, _memoryOffset ),
        "pbr::FontStorage::StagingBuffer",
        "Can't bind memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* data;

    result = renderer.MapMemory ( data,
        _memory,
        _memoryOffset,
        "pbr::FontStorage::StagingBuffer::Init",
        "Can't map memory"
    );

    if ( !result ) [[unlikely]]
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
    }

    if ( _memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _memory, _memoryOffset );
    _memory = VK_NULL_HANDLE;
    _memoryOffset = 0U;
}

void FontStorage::StagingBuffer::AddGlyph ( GlyphStorage& glyphs, char32_t character ) noexcept
{
    _bindings.push_back (
        {
            ._glyphs = &glyphs,
            ._character = character
        }
    );
}

void FontStorage::StagingBuffer::BindGlyphs ( uint16_t heapResource ) noexcept
{
    for ( auto const &bind : _bindings )
        bind._glyphs->at ( bind._character )._atlas = heapResource;

    _bindings.clear ();
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
    _hasNewGlyphs = false;
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

        AV_SET_VULKAN_OBJECT_NAME ( device, resource._image, VK_OBJECT_TYPE_IMAGE, "Font storage atlas page #%u", i )
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

        if ( auto const idx = resourceHeap.RegisterUISampledImage ( device, resource._view ); idx ) [[likely]]
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

        if ( page._memory == VK_NULL_HANDLE ) [[unlikely]]
            continue;

        renderer.FreeMemory ( page._memory, page._memoryOffset );
    }

    _pages.clear ();
    _pages.shrink_to_fit ();
}

//----------------------------------------------------------------------------------------------------------------------

FontStorage::FontStorage ( ResourceHeap& resourceHeap ) noexcept:
    _resourceHeap ( resourceHeap )
{
    // NOTHING
}

bool FontStorage::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    return CheckFTResult ( FT_Init_FreeType ( &_library ), "pbr::FontStorage::Init", "Can't init FreeType" ) &&
        MakeTransparentGlyph ( renderer );
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

    if ( !result ) [[unlikely]]
        return;

    _library = nullptr;
    _fontResources.clear ();
    _stringHeap.clear ();
}

std::optional<FontStorage::FontLock> FontStorage::GetFont ( std::string_view font, uint32_t size ) noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine.
    constexpr size_t magic = 0x9E3779B9U;

    static std::hash<uint32_t> const hashInteger {};
    static std::hash<std::string_view> const hashString {};

    FontHash hash = 0U;
    hash ^= hashString ( font ) + magic + ( hash << 6U ) + ( hash >> 2U );
    hash ^= ( hashInteger ( size ) + magic + ( hash << 6U ) + ( hash >> 2U ) );

    {
        // Hoping for the best: the font already presents.
        std::shared_lock sharedLock ( _mutex );

        if ( auto const findResult = _fonts.find ( hash ); findResult != _fonts.end () ) [[likely]]
        {
            return std::optional<FontStorage::FontLock> { FontLock ( findResult, std::move ( sharedLock ) ) };
        }
    }

    {
        // It's needed to create new font.
        std::unique_lock const exclusiveLock ( _mutex );

        // There is a chance that somebody created target font in other thread.
        if ( !_fonts.contains ( hash ) )
        {
            // Nah. Nobody created the font.
            if ( !MakeFont ( hash, font, size ) ) [[unlikely]]
            {
                return std::nullopt;
            }
        }
    }

    // There is a chance that other thread could insert items into '_fonts' storage. It's needed to find font again.
    std::shared_lock sharedLock ( _mutex );
    return std::optional<FontStorage::FontLock> { FontLock ( _fonts.find ( hash ), std::move ( sharedLock ) ) };
}

FontStorage::GlyphInfo const &FontStorage::GetGlyphInfo ( android_vulkan::Renderer &renderer,
    Font font,
    char32_t character
) noexcept
{
    FontData &fontData = font->second;
    GlyphStorage &glyphs = fontData._glyphs;

    if ( auto const glyph = glyphs.find ( character ); glyph != glyphs.cend () )
        return glyph->second;

    return EmbedGlyph ( renderer,
        glyphs,
        fontData._face,
        fontData._fontSize,
        fontData._metrics._ascend,
        character
    );
}

void FontStorage::GetStringMetrics ( StringMetrics &result,
    std::string_view font,
    uint32_t size,
    std::u32string_view string
) noexcept
{
    result.clear ();

    if ( string.empty () )
        return;

    auto const fontInfo = GetFont ( font, size );

    if ( !fontInfo ) [[unlikely]]
        return;

    result.reserve ( string.size () + 1U );

    Font const f = fontInfo->_font;
    int32_t p = 0U;
    char32_t prevSymbol = 0;

    constexpr size_t firstSymbolOffsetX = 1U;
    constexpr size_t notFirstSymbolOffsetX = 0U;
    int32_t offsetX[] = { 0, 0 };
    size_t isFirst = firstSymbolOffsetX;

    FontData const &fontData = f->second;
    FT_Face face = fontData._face;

    GlyphStorage const &glyphs = fontData._glyphs;
    auto const end = glyphs.cend ();

    for ( ; !string.empty (); string = string.substr ( 1U ) )
    {
        char32_t const symbol = string.front ();
        p += GetKerning ( f, std::exchange ( prevSymbol, symbol ), symbol );

        if ( auto const glyph = glyphs.find ( symbol ); glyph != end ) [[likely]]
        {
            GlyphInfo const& gi = glyph->second;
            offsetX[ firstSymbolOffsetX ] = gi._offsetX;
            p += offsetX[ std::exchange ( isFirst, notFirstSymbolOffsetX ) ];
            result.push_back ( static_cast<float> ( std::exchange ( p, p + gi._advance ) ) );
            continue;
        }

        bool const status = CheckFTResult (
            FT_Load_Char ( face, static_cast<FT_ULong> ( symbol ), FT_LOAD_BITMAP_METRICS_ONLY ),
            "pbr::FontStorage::GetStringMetrics",
            "Can't get glyph metrics"
        );

        if ( !status ) [[unlikely]]
        {
            result.clear ();
            AV_ASSERT ( false )
            return;
        }

        FT_GlyphSlot const slot = face->glyph;
        offsetX[ firstSymbolOffsetX ] = static_cast<int32_t> ( slot->metrics.horiBearingX ) >> 6U;
        p += offsetX[ std::exchange ( isFirst, notFirstSymbolOffsetX ) ];

        result.push_back (
            static_cast<float> ( std::exchange ( p, p + ( static_cast<int32_t> ( slot->advance.x ) >> 6U ) ) )
        );
    }

    result.push_back ( static_cast<float> ( p ) );
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
        Line const &line = _fullStagingBuffers.front ()._startLine;
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

    if ( _transparentGlyph._atlas == ResourceHeap::INVALID_UI_IMAGE ) [[unlikely]]
        _transparentGlyph._atlas = pages->_heapResource;

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

FontStorage::PixelFontMetrics const &FontStorage::GetFontPixelMetrics ( Font font ) noexcept
{
    return font->second._metrics;
}

int32_t FontStorage::GetKerning ( Font font, char32_t left, char32_t right ) noexcept
{
    FT_Face face = font->second._face;

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
    _atlas.Destroy ( renderer, _resourceHeap );

    auto const clear = [ & ] ( auto &buffers ) noexcept {
        for ( auto &buffer : buffers )
            buffer.Destroy ( renderer );

        buffers.clear ();
    };

    clear ( _activeStagingBuffer );
    clear ( _freeStagingBuffers );
    clear ( _fullStagingBuffers );

    std::lock_guard const lock ( _mutex );
    _fonts.clear ();
}

FontStorage::GlyphInfo const &FontStorage::EmbedGlyph ( android_vulkan::Renderer &renderer,
    GlyphStorage &glyphs,
    FT_Face face,
    uint32_t fontSize,
    int32_t ascend,
    char32_t character
) noexcept
{
    static GlyphInfo const nullGlyph {};
    auto query = GetStagingBuffer ( renderer );

    if ( !query ) [[unlikely]]
        return nullGlyph;

    StagingBuffer* stagingBuffer = query.value ();

    bool const result = CheckFTResult ( FT_Load_Char ( face, static_cast<FT_ULong> ( character ), FT_LOAD_RENDER ),
        "pbr::android::FontStorage::EmbedGlyph",
        "Can't get glyph bitmap"
    );

    if ( !result ) [[unlikely]]
        return nullGlyph;

    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap const &bm = slot->bitmap;

    auto const rows = static_cast<uint16_t> ( bm.rows );
    auto const width = static_cast<size_t> ( bm.width );
    auto const advance = static_cast<int32_t> ( slot->advance.x ) >> 6U;

    FT_Glyph_Metrics const &metrics = slot->metrics;
    auto const offsetX = static_cast<int32_t> ( metrics.horiBearingX ) >> 6U;
    auto const offsetYFactor = static_cast<int32_t> ( metrics.height - metrics.horiBearingY ) >> 6U;
    int32_t const offsetY = ascend - static_cast<int32_t> ( rows ) + offsetYFactor;

    if ( ( rows == 0U ) | ( width == 0U ) )
    {
        auto const status = glyphs.insert (
            std::make_pair ( character,

                GlyphInfo
                {
                    ._topLeft = _transparentGlyph._topLeft,
                    ._bottomRight = _transparentGlyph._bottomRight,
                    ._atlas = _transparentGlyph._atlas,
                    ._width = 0,
                    ._height = 0,
                    ._advance = advance,
                    ._offsetX = 0,
                    ._offsetY = 0
                }
            )
        );

        return status.first->second;
    }

    if ( ( rows > FONT_ATLAS_RESOLUTION ) | ( static_cast<uint16_t> ( width ) > FONT_ATLAS_RESOLUTION ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "FontStorage::EmbedGlyph - Font size is way too big: %u", fontSize );
        return nullGlyph;
    }

    auto const toRight = static_cast<uint16_t> ( width - 1U );
    uint16_t const toBottom = rows - 1U;

    uint16_t lineHeight = stagingBuffer->_endLine._height;

    // Move position on one pixel right to not overwrite previously rendered glyph last column.
    uint16_t left = static_cast<uint16_t> ( stagingBuffer->_endLine._x + static_cast<uint16_t> ( lineHeight > 0U ) );
    uint16_t top = stagingBuffer->_endLine._y;

    auto const goToNewLine = [ & ] () noexcept {
        if ( stagingBuffer->_state == StagingBuffer::eState::FirstLine )
            stagingBuffer->_startLine._height = stagingBuffer->_endLine._height;

        stagingBuffer->_state = StagingBuffer::eState::FullLinePresent;
        lineHeight = 0U;

        top += stagingBuffer->_endLine._height;
        left = 0U;
    };

    if ( left >= FONT_ATLAS_RESOLUTION ) [[unlikely]]
        goToNewLine ();

    auto const completeStagingBuffer = [ & ]() noexcept -> bool {
        auto begin = _activeStagingBuffer.begin ();

        if ( !begin->_hasNewGlyphs ) [[likely]]
        {
            begin->Reset ();
        }
        else
        {
            _fullStagingBuffers.splice ( _fullStagingBuffers.cend (), _activeStagingBuffer, begin );
            query = GetStagingBuffer ( renderer );

            if ( !query ) [[unlikely]]
                return false;

            stagingBuffer = query.value ();
        }

        lineHeight = 0U;
        top = 0U;
        left = 0U;
        return true;
    };

    if ( top >= FONT_ATLAS_RESOLUTION && !completeStagingBuffer () ) [[unlikely]]
        return nullGlyph;

    uint16_t right = static_cast<uint16_t> ( left + toRight );
    uint16_t bottom = static_cast<uint16_t> ( top + toBottom );

    if ( right >= FONT_ATLAS_RESOLUTION ) [[unlikely]]
    {
        goToNewLine ();
        bottom = static_cast<uint16_t> ( top + toBottom );
        right = toRight;
    }

    if ( bottom >= FONT_ATLAS_RESOLUTION ) [[unlikely]]
    {
        if ( !completeStagingBuffer () ) [[unlikely]]
            return nullGlyph;

        bottom = toBottom;
        right = toRight;
    }

    stagingBuffer->_endLine =
    {
        ._height = std::max ( lineHeight, rows ),
        ._x = right,
        ._y = top
    };

    stagingBuffer->_hasNewGlyphs = true;

    constexpr auto res = static_cast<size_t> ( FONT_ATLAS_RESOLUTION );
    uint8_t* data = stagingBuffer->_data + static_cast<size_t> ( left ) + res * static_cast<size_t> ( top );

    auto const* raster = static_cast<uint8_t const*> ( bm.buffer );
    auto const pitch = static_cast<ptrdiff_t> ( bm.pitch );

    // 'pitch' is positive when glyph image is stored from top to bottom line flow.
    // 'pitch' is negative when glyph image is stored from bottom to top line flow.
    // It's needed to add 'pitch' signed value to 'buffer' to go to the next line in normal top to bottom line flow.

    for ( uint16_t row = 0U; row < rows; ++row )
    {
        std::memcpy ( data, raster, static_cast<size_t> ( width ) );
        raster += pitch;
        data += res;
    }

    stagingBuffer->AddGlyph ( glyphs, character );

    auto const status = glyphs.insert (
        std::make_pair ( character,

            GlyphInfo
            {
                ._topLeft = PixToUV ( left, top ),
                ._bottomRight = PixToUV ( right + 1U, bottom + 1U ),
                ._atlas = ResourceHeap::INVALID_UI_IMAGE,
                ._width = static_cast<int32_t> ( width ),
                ._height = static_cast<int32_t> ( rows ),
                ._advance = advance,
                ._offsetX = offsetX,
                ._offsetY = offsetY
            }
        )
    );

    return status.first->second;
}

std::optional<FontStorage::StagingBuffer*> FontStorage::GetStagingBuffer (
    android_vulkan::Renderer &renderer
) noexcept
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

    StagingBuffer &stagingBuffer = _activeStagingBuffer.emplace_front ();

    if ( stagingBuffer.Init ( renderer ) )
        return &stagingBuffer;

    return std::nullopt;
}

bool FontStorage::MakeFont ( FontHash hash, std::string_view font, uint32_t size ) noexcept
{
    FontResource* fontResource;

    if ( auto const findResult = _fontResources.find ( font ); findResult != _fontResources.cend () ) [[likely]]
    {
        fontResource = &findResult->second;
    }
    else
    {
        android_vulkan::File fontAsset ( font );

        if ( !fontAsset.LoadContent () ) [[unlikely]]
            return false;

        auto const status = _fontResources.emplace ( std::string_view ( _stringHeap.emplace_front ( font ) ),
            FontResource
            {
                ._fontAsset = std::move ( fontAsset.GetContent () ),
                ._metrics {}
            }
        );

        fontResource = &status.first->second;
    }

    std::vector<uint8_t> const &fontAsset = fontResource->_fontAsset;
    FT_Face face;

    bool const result = CheckFTResult (
        FT_New_Memory_Face ( _library, fontAsset.data (), static_cast<FT_Long> ( fontAsset.size () ), 0, &face ),
        "pbr::FontStorage::MakeFont",
        "Can't load face"
    );

    if ( !result ) [[unlikely]]
        return false;

    EMFontMetrics &metrics = fontResource->_metrics;

    if ( metrics._contentAreaHeight == 0.0 ) [[unlikely]]
    {
        auto emMetrics = ResolveEMFontMetrics ( face );

        if ( !emMetrics ) [[unlikely]]
            return false;

        metrics = std::move ( *emMetrics );
    }

    auto const s = static_cast<FT_UInt> ( size );

    if ( !CheckFTResult ( FT_Set_Pixel_Sizes ( face, s, s ), "pbr::FontStorage::MakeFont", "Can't set size" ) )
    {
        [[unlikely]]
        return false;
    }

    auto const sz = static_cast<double> ( size );

    _fonts.emplace ( hash,
        FontData
        {
            ._face = face,
            ._fontSize = size,
            ._glyphs = {},

            ._metrics
            {
                ._ascend = static_cast<int32_t> ( sz * metrics._ascend ),
                ._baselineToBaseline = static_cast<int32_t> ( sz * metrics._baselineToBaseline ),
                ._contentAreaHeight = static_cast<int32_t> ( sz * metrics._contentAreaHeight )
            }
        }
    );

    return true;
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
        ._atlas = ResourceHeap::INVALID_UI_IMAGE,
        ._width = 1,
        ._height = 1,
        ._advance = 0,
        ._offsetY = 0
    };

    StagingBuffer &stagingBuffer = *query.value ();
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

    auto const transferComplex = [ & ] ( StagingBuffer const &b ) noexcept {
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
        StagingBuffer &b = *begin;
        transferComplex ( b );
        b.Reset ();

        ++begin;
    }

    for ( ; begin != end; ++begin )
    {
        // Rest full buffers contain only one area: entire atlas layer.
        StagingBuffer &b = *begin;
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
    StagingBuffer &b = *begin;
    transferComplex ( b );
    _freeStagingBuffers.splice ( _freeStagingBuffers.cend (), _activeStagingBuffer, begin );

    _atlas._line = b._endLine;

    b._startLine =
    {
        ._height = b._endLine._height,
        ._x = static_cast<uint16_t> ( b._endLine._x + 1U ),
        ._y = b._endLine._y
    };

    b._state = StagingBuffer::eState::FirstLine;

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

bool FontStorage::CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept
{
    if ( result == FT_Err_Ok ) [[likely]]
        return true;

    android_vulkan::LogError ( "%s - %s. Error: %s.", from, message, FT_Error_String ( result ) );
    return false;
}

android_vulkan::Half2 FontStorage::PixToUV ( uint32_t x, uint32_t y ) noexcept
{
    constexpr float pix2UV = 1.0F / static_cast<float> ( FONT_ATLAS_RESOLUTION );
    constexpr float threshold = pix2UV * 0.25F;
    constexpr GXVec2 pointSamplerUVThreshold ( threshold, threshold );

    GXVec2 a {};
    a.Sum ( pointSamplerUVThreshold, pix2UV, GXVec2 ( static_cast<float> ( x ), static_cast<float> ( y ) ) );

    return android_vulkan::Half2 ( a );
}

std::optional<FontStorage::EMFontMetrics> FontStorage::ResolveEMFontMetrics ( FT_Face face ) noexcept
{
    // Based on ideas from Skia:
    // https://source.chromium.org/chromium/chromium/src/+/main:third_party/skia/src/ports/SkFontHost_FreeType.cpp;l=1559;drc=f39c57f31413abcb41d3068cfb2c7a1718003cc5;bpv=0;bpt=1

    auto const em = static_cast<FT_UInt> ( face->units_per_EM );
    auto const invEM = 1.0 / static_cast<double> ( em );

    EMFontMetrics metrics
    {
        ._ascend = invEM * static_cast<double> ( face->ascender ),
        ._baselineToBaseline = invEM * static_cast<double> ( face->height ),

        // FreeType provides 'face->descender' as negative value.
        ._contentAreaHeight = invEM * static_cast<double> ( face->ascender - face->descender )
    };

    if ( auto const* os2 = static_cast<TT_OS2 const*> ( FT_Get_Sfnt_Table ( face, ft_sfnt_os2 ) ); os2 )
    {
        if ( FT_Short const xHeight = os2->sxHeight; xHeight != 0 )
        {
            metrics._xHeight = invEM * static_cast<double> ( xHeight );
            return std::optional<FontStorage::EMFontMetrics> { std::move ( metrics ) };
        }
    }

    // Trying to use actual 'x' glyph to get metrics.
    // Half of EM size. Blind guess. Better than nothing.
    auto const defaultXHeight = invEM * static_cast<double> ( em >> 1U );

    if ( !( face->face_flags & FT_FACE_FLAG_SCALABLE ) )
    {
        metrics._xHeight = defaultXHeight;
        return std::optional<FontStorage::EMFontMetrics> { std::move ( metrics ) };
    }

    bool result = CheckFTResult ( FT_Set_Pixel_Sizes ( face, em, em ),
        "pbr::FontStorage::ResolveEMFontMetrics",
        "Can't set size"
    );

    if ( !result )
    {
        [[unlikely]]
        return std::nullopt;
    }

    result = FT_Load_Char ( face, static_cast<FT_ULong> ( U'x' ), FT_LOAD_BITMAP_METRICS_ONLY ) == FT_Err_Ok &&
        face->glyph->format == FT_GLYPH_FORMAT_OUTLINE;

    if ( !result )
    {
        metrics._xHeight = defaultXHeight;
        return std::optional<FontStorage::EMFontMetrics> { std::move ( metrics ) };
    }

    FT_Pos maxY = 0;
    FT_Outline const &outline = face->glyph->outline;

    for ( FT_Vector const &p : std::span<FT_Vector const> ( outline.points, static_cast<size_t> ( outline.n_points ) ) )
        maxY = std::max ( maxY, p.y );

    double const cases[] = { defaultXHeight, invEM * static_cast<double> ( maxY >> 6 ) };
    metrics._xHeight = cases[ static_cast<size_t> ( maxY > 0 ) ];

    return std::optional<FontStorage::EMFontMetrics> { std::move ( metrics ) };
}

} // namespace pbr
