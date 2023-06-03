#include <pbr/font_storage.h>
#include <file.h>
#include <logger.h>
#include <vulkan_utils.h>


namespace pbr {

namespace {

constexpr uint32_t SPECIAL_GLYPH_ATLAS_LAYER = 0U;

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

    if ( _memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _memory, _memoryOffset );
        _memory = VK_NULL_HANDLE;
        _memoryOffset = 0U;
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::FontStorage::StagingBuffer::_memory" )
    }

    if ( _buffer == VK_NULL_HANDLE )
        return;

    vkDestroyBuffer ( renderer.GetDevice (), _buffer, nullptr );
    _buffer = VK_NULL_HANDLE;
    AV_UNREGISTER_BUFFER ( "pbr::FontStorage::StagingBuffer::_buffer" )
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] bool FontStorage::Atlas::AddLayer ( android_vulkan::Renderer &/*renderer*/,
    VkCommandBuffer /*commandBuffer*/
) noexcept
{
    // TODO
    return true;
}

void FontStorage::Atlas::Destroy ( android_vulkan::Renderer &/*renderer*/ ) noexcept
{
    // TODO
}

void FontStorage::Atlas::SetResolution ( VkExtent2D const &resolution ) noexcept
{
    _side = std::min ( resolution.width, resolution.height );
}

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::Init (  android_vulkan::Renderer &renderer, VkExtent2D const &nativeViewport ) noexcept
{
    _atlas.SetResolution ( nativeViewport );
    _pixToUV = 1.0F / static_cast<float> ( _atlas._side );

    float const h = _pixToUV * 0.5F;
    _halfPixelUV._data[ 0U ] = h;
    _halfPixelUV._data[ 1U ] = h;

    return MakeSpecialGlyphs ( renderer ) &&
        CheckFTResult ( FT_Init_FreeType ( &_library ), "pbr::FontStorage::Init", "Can't init FreeType" );
}

void FontStorage::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _atlas.Destroy ( renderer );

    for ( auto& buffer : _freeStagingBuffers )
        buffer.Destroy ( renderer );

    for ( auto& buffer : _fullStagingBuffers )
        buffer.Destroy ( renderer );

    _freeStagingBuffers.clear ();
    _fullStagingBuffers.clear ();

    if ( !_library )
        return;

    bool const result = CheckFTResult ( FT_Done_FreeType ( _library ),
        "pbr::FontStorage::Destroy",
        "Can't close FreeType"
    );

    if ( !result )
        return;

    _library = nullptr;
    _fonts.clear ();
    _fontResources.clear ();
    _stringHeap.clear ();
}

bool FontStorage::UploadGPUData ( android_vulkan::Renderer &/*renderer*/, VkCommandBuffer /*commandBuffer*/ ) noexcept
{
    // TODO
    return true;
}

std::optional<FontStorage::Font> FontStorage::GetFont ( std::string_view font, uint32_t size ) noexcept
{
    auto fontResource = GetFontResource ( font );

    if ( !fontResource )
        return std::nullopt;

    FontResource* f = *fontResource;

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

    android_vulkan::File fontAsset ( font );
    [[maybe_unused]] bool const result = fontAsset.LoadContent ();

    auto status = _fonts.emplace ( hash,
        FontData
        {
            ._fontResource = f,
            ._fontSize = size,
            ._glyphs = {}
        }
    );

    return status.first;
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
    auto const s = static_cast<FT_UInt> ( fontSize );

    if ( !CheckFTResult ( FT_Set_Pixel_Sizes ( face, s, s ), "pbr::FontStorage::EmbedGlyph", "Can't set size" ) )
        return nullGlyph;

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
                ._atlasLayer = SPECIAL_GLYPH_ATLAS_LAYER,
                ._topLeft = _transparentGlyphUV,
                ._bottomRight = _transparentGlyphUV,
                ._width = static_cast<int32_t> ( width ),
                ._height = static_cast<int32_t> ( rows ),
                ._advance = static_cast<int32_t> ( slot->advance.x ) >> 6,
                ._offsetY = static_cast<int32_t> ( slot->metrics.horiBearingY - slot->metrics.height ) >> 6
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

    uint32_t lineHeight =  stagingBuffer->_lineHeight;

    // Move position on one pixel right to not overwrite previously rendered glyph last column.
    uint32_t left = stagingBuffer->_endX + static_cast<uint32_t> ( stagingBuffer->_endX > 0U );
    uint32_t top = stagingBuffer->_endY;

    auto const goToNewLine = [ & ]() noexcept {
        if ( stagingBuffer->_state == StagingBuffer::eState::FirstLine )
            stagingBuffer->_firstLineHeight = stagingBuffer->_lineHeight;

        stagingBuffer->_state = StagingBuffer::eState::FullLinePresent;
        lineHeight = 0U;

        top += stagingBuffer->_lineHeight;
        left = 0U;
    };

    if ( left >= side )
        goToNewLine ();

    auto const completeStagingBuffer = [ & ]() noexcept -> bool {
        _fullStagingBuffers.splice ( _fullStagingBuffers.cend (),
            _freeStagingBuffers,
            _freeStagingBuffers.cbegin ()
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

    stagingBuffer->_lineHeight = std::max ( lineHeight, rows );
    stagingBuffer->_endX = right;
    stagingBuffer->_endY = top;
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

    auto const status = glyphs.emplace ( character,
        GlyphInfo
        {
            ._atlasLayer = _atlas._layers + static_cast<uint32_t> ( _fullStagingBuffers.size () ),
            ._topLeft = PixToUV ( left, top ),
            ._bottomRight = PixToUV ( right, bottom ),
            ._width = static_cast<int32_t> ( width ),
            ._height = static_cast<int32_t> ( rows ),
            ._advance = static_cast<int32_t> ( slot->advance.x ) >> 6,
            ._offsetY = static_cast<int32_t> ( slot->metrics.horiBearingY - slot->metrics.height ) >> 6
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
    if ( !_freeStagingBuffers.empty () )
        return &_freeStagingBuffers.front ();

    StagingBuffer& stagingBuffer = _freeStagingBuffers.emplace_front ();

    if ( stagingBuffer.Init ( renderer, _atlas._side ) )
        return &stagingBuffer;

    return std::nullopt;
}

bool FontStorage::MakeSpecialGlyphs ( android_vulkan::Renderer &renderer ) noexcept
{
    auto query = GetStagingBuffer ( renderer );

    if ( !query )
        return false;

    _opaqueGlyphUV = PixToUV ( 0U, 0U );
    _transparentGlyphUV = PixToUV ( 1U, 0U );

    StagingBuffer &stagingBuffer = *query.value ();
    stagingBuffer._data[ 0U ] = std::numeric_limits<uint8_t>::max ();
    stagingBuffer._data[ 1U ] = 0U;

    stagingBuffer._endX = 1U;
    stagingBuffer._lineHeight = 1U;

    return true;
}

GXVec2 FontStorage::PixToUV ( uint32_t x, uint32_t y ) const noexcept
{
    GXVec2 result {};
    result.Sum ( _halfPixelUV, _pixToUV, GXVec2 ( static_cast<float> ( x ), static_cast<float> ( y ) ) );
    return result;
}

bool FontStorage::CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept
{
    if ( result == FT_Err_Ok )
        return true;

    android_vulkan::LogError ( "%s - %s. Error: %s.", from, message, FT_Error_String ( result ) );
    return false;
}

} // namespace pbr
