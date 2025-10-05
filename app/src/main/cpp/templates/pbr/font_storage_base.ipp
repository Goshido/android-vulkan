#ifndef PBR_FONT_STORAGE_BASE_IPP
#define PBR_FONT_STORAGE_BASE_IPP


#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

template <typename G, typename S>
FontStorageBase<G, S>::FontLock::FontLock ( Font font, std::shared_lock<std::shared_mutex> &&lock ) noexcept:
    _font ( font ),
    _lock ( std::move ( lock ) )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

template <typename G, typename S>
std::optional<typename FontStorageBase<G, S>::FontLock> FontStorageBase<G, S>::GetFont ( std::string_view font,
    uint32_t size
) noexcept
{
    FontHash const hash = MakeFontHash ( font, size );

    {
        // Hoping for the best: the font already presents.
        std::shared_lock sharedLock ( _mutex );

        if ( auto const findResult = _fonts.find ( hash ); findResult != _fonts.end () ) [[likely]]
        {
            return std::optional<FontLock> { FontLock ( findResult, std::move ( sharedLock ) ) };
        }
    }

    {
        // It's needed to create new font.
        std::unique_lock const exclusiveLock ( _mutex );

        // There is a chance that somebody created target font in other thread.
        if ( !_fonts.contains ( hash ) && !MakeFont ( hash, font, size ) ) [[unlikely]]
        {
            // Nah. Nobody created the font.
            return std::nullopt;
        }
    }

    // There is a chance that other thread could insert items into font storage. It's needed to find font again.
    return std::optional<FontLock> { FontLock ( _fonts.find ( hash ), std::shared_lock ( _mutex ) ) };
}

template <typename G, typename S>
G const &FontStorageBase<G, S>::GetGlyphInfo ( android_vulkan::Renderer &renderer,
    Font font,
    char32_t character
) noexcept
{
    FontData &fontData = font->second;
    GlyphStorage<G> &glyphs = fontData._glyphs;

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

template <typename G, typename S>
void FontStorageBase<G, S>::GetStringMetrics ( StringMetrics &result,
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

    GlyphStorage<G> const &glyphs = fontData._glyphs;
    auto const end = glyphs.cend ();

    for ( ; !string.empty (); string = string.substr ( 1U ) )
    {
        char32_t const symbol = string.front ();
        p += GetKerning ( f, std::exchange ( prevSymbol, symbol ), symbol );

        if ( auto const glyph = glyphs.find ( symbol ); glyph != end ) [[likely]]
        {
            G const &gi = glyph->second;
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

        FT_GlyphSlot slot = face->glyph;
        offsetX[ firstSymbolOffsetX ] = static_cast<int32_t> ( slot->metrics.horiBearingX ) >> 6U;
        p += offsetX[ std::exchange ( isFirst, notFirstSymbolOffsetX ) ];

        result.push_back (
            static_cast<float> ( std::exchange ( p, p + ( static_cast<int32_t> ( slot->advance.x ) >> 6U ) ) )
        );
    }

    result.push_back ( static_cast<float> ( p ) );
}

template <typename G, typename S>
FontStorageBase<G, S>::PixelFontMetrics const &FontStorageBase<G, S>::GetFontPixelMetrics ( Font font ) noexcept
{
    return font->second._metrics;
}

template <typename G, typename S>
int32_t FontStorageBase<G, S>::GetKerning ( Font font, char32_t left, char32_t right ) noexcept
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

        "pbr::FontStorageBase::GetKerning",
        "Can't resolve kerning"
    );

    int32_t const cases[] = { 0, static_cast<int32_t> ( delta.x >> 6 ) };
    return cases[ static_cast<size_t> ( status ) ];
}

template <typename G, typename S>
bool FontStorageBase<G, S>::InitBase () noexcept
{
    return CheckFTResult ( FT_Init_FreeType ( &_library ), "pbr::FontStorageBase::InitBase", "Can't init FreeType" );
}

template <typename G, typename S>
void FontStorageBase<G, S>::DestroyBase ( android_vulkan::Renderer &renderer ) noexcept
{
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

    bool const status = _library &&

        CheckFTResult ( FT_Done_FreeType ( std::exchange ( _library, nullptr ) ),
            "pbr::FontStorageBase::DestroyBase",
            "Can't close FreeType"
        );

    if ( !status ) [[unlikely]]
        return;

    _fontResources.clear ();
    _stringHeap.clear ();
}

template <typename G, typename S>
std::optional<S*> FontStorageBase<G, S>::GetStagingBuffer ( android_vulkan::Renderer &renderer ) noexcept
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

    if ( S &stagingBuffer = _activeStagingBuffer.emplace_front (); stagingBuffer.Init ( renderer ) )
        return &stagingBuffer;

    return std::nullopt;
}

template <typename G, typename S>
android_vulkan::Half2 FontStorageBase<G, S>::PixToUV ( uint32_t x, uint32_t y ) noexcept
{
    constexpr float pix2UV = 1.0F / static_cast<float> ( FONT_ATLAS_RESOLUTION );
    constexpr float threshold = pix2UV * 0.25F;
    constexpr GXVec2 pointSamplerUVThreshold ( threshold, threshold );

    GXVec2 a {};
    a.Sum ( pointSamplerUVThreshold, pix2UV, GXVec2 ( static_cast<float> ( x ), static_cast<float> ( y ) ) );
    return android_vulkan::Half2 ( a );
}

template <typename G, typename S>
bool FontStorageBase<G, S>::MakeFont ( FontHash hash, std::string_view font, uint32_t size ) noexcept
{
    MakeFontResult const result = MakeFontInternal ( font, size );

    if ( !result ) [[unlikely]]
        return false;

    auto const sz = static_cast<double> ( size );
    EMFontMetrics const &metrics = *result->_metrics;

    _fonts.emplace ( hash,
        FontData
        {
            ._face = result->_face,
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

template <typename G, typename S>
G const &FontStorageBase<G, S>::EmbedGlyph ( android_vulkan::Renderer &renderer,
    GlyphStorage<G> &glyphs,
    FT_Face face,
    uint32_t fontSize,
    int32_t ascend,
    char32_t character
) noexcept
{
    static G const nullGlyph {};
    auto query = GetStagingBuffer ( renderer );

    if ( !query ) [[unlikely]]
        return nullGlyph;

    S* stagingBuffer = query.value ();

    bool const result = CheckFTResult ( FT_Load_Char ( face, static_cast<FT_ULong> ( character ), FT_LOAD_RENDER ),
        "pbr::FontStorageBase::EmbedGlyph",
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
        return glyphs.insert (
            std::make_pair ( character,

                G
                {
                    ._topLeft = _transparentGlyph._topLeft,
                    ._bottomRight = _transparentGlyph._bottomRight,
                    ._pageID = _transparentGlyph._pageID,
                    ._width = 0,
                    ._height = 0,
                    ._advance = advance,
                    ._offsetX = 0,
                    ._offsetY = 0
                }
            )
        ).first->second;
    }

    if ( ( rows > FONT_ATLAS_RESOLUTION ) | ( static_cast<uint16_t> ( width ) > FONT_ATLAS_RESOLUTION ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::FontStorageBase::EmbedGlyph - Font size is way too big: %u", fontSize );
        return nullGlyph;
    }

    auto const toRight = static_cast<uint16_t> ( width - 1U );
    uint16_t const toBottom = rows - 1U;

    uint16_t lineHeight = stagingBuffer->_endLine._height;

    // Move position on one pixel right to not overwrite previously rendered glyph last column.
    auto left = static_cast<uint16_t> ( stagingBuffer->_endLine._x + static_cast<uint16_t> ( lineHeight > 0U ) );
    uint16_t top = stagingBuffer->_endLine._y;

    auto const goToNewLine = [ & ] () noexcept {
        if ( stagingBuffer->_state == S::eState::FirstLine )
            stagingBuffer->_startLine._height = stagingBuffer->_endLine._height;

        stagingBuffer->_state = S::eState::FullLinePresent;
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

    auto right = static_cast<uint16_t> ( left + toRight );
    auto bottom = static_cast<uint16_t> ( top + toBottom );

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
        std::memcpy ( data, raster, width );
        raster += pitch;
        data += res;
    }

    return InsertGlyph ( glyphs,
        *stagingBuffer,
        character,
        offsetX,
        offsetY,
        advance,
        static_cast<int32_t> ( width ),
        static_cast<int32_t> ( rows ),
        PixToUV ( left, top ),
        PixToUV ( right + 1U, bottom + 1U )
    );
}

template <typename G, typename S>
FontStorageBase<G, S>::MakeFontResult FontStorageBase<G, S>::MakeFontInternal ( std::string_view font,
    uint32_t size
) noexcept
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
            return {};

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
    MakeFontInfo fontInfo;

    bool result = CheckFTResult (
        FT_New_Memory_Face ( _library,
            fontAsset.data (),
            static_cast<FT_Long> ( fontAsset.size () ),
            0,
            &fontInfo._face
        ),

        "pbr::FontStorageBase::MakeFontInternal",
        "Can't load face"
    );

    if ( !result ) [[unlikely]]
        return {};

    fontInfo._metrics = &fontResource->_metrics;

    if ( fontInfo._metrics->_contentAreaHeight == 0.0 ) [[unlikely]]
    {
        auto emMetrics = ResolveEMFontMetrics ( fontInfo._face );

        if ( !emMetrics ) [[unlikely]]
            return {};

        *fontInfo._metrics = std::move ( *emMetrics );
    }

    auto const s = static_cast<FT_UInt> ( size );

    result = CheckFTResult ( FT_Set_Pixel_Sizes ( fontInfo._face, s, s ),
        "pbr::FontStorageBase::MakeFontInternal",
        "Can't set size"
    );

    if ( !result ) [[unlikely]]
        return {};

    return MakeFontResult { std::move ( fontInfo ) };
}

template <typename G, typename S>
bool FontStorageBase<G, S>::CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept
{
    if ( result == FT_Err_Ok ) [[likely]]
        return true;

    android_vulkan::LogError ( "%s - %s. Error: %s.", from, message, FT_Error_String ( result ) );
    return false;
}

template <typename G, typename S>
FontStorageBase<G, S>::FontHash FontStorageBase<G, S>::MakeFontHash ( std::string_view font, uint32_t size ) noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine.
    constexpr size_t magic = 0x9E3779B9U;

    static std::hash<uint32_t> const hashInteger {};
    static std::hash<std::string_view> const hashString {};

    FontHash hash = 0U;
    hash ^= hashString ( font ) + magic + ( hash << 6U ) + ( hash >> 2U );
    return hash ^ hashInteger ( size ) + magic + ( hash << 6U ) + ( hash >> 2U );
}

template <typename G, typename S>
std::optional<typename FontStorageBase<G, S>::EMFontMetrics> FontStorageBase<G, S>::ResolveEMFontMetrics (
    FT_Face face
) noexcept
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
            return std::optional<FontStorageBase::EMFontMetrics> { std::move ( metrics ) };
        }
    }

    // Trying to use actual 'x' glyph to get metrics.
    // Half of EM size. Blind guess. Better than nothing.
    auto const defaultXHeight = invEM * static_cast<double> ( em >> 1U );

    if ( !( face->face_flags & FT_FACE_FLAG_SCALABLE ) )
    {
        metrics._xHeight = defaultXHeight;
        return std::optional<FontStorageBase::EMFontMetrics> { std::move ( metrics ) };
    }

    bool result = CheckFTResult ( FT_Set_Pixel_Sizes ( face, em, em ),
        "pbr::FontStorageBase::ResolveEMFontMetrics",
        "Can't set size"
    );

    if ( !result ) [[unlikely]]
        return std::nullopt;

    result = FT_Load_Char ( face, static_cast<FT_ULong> ( U'x' ), FT_LOAD_BITMAP_METRICS_ONLY ) == FT_Err_Ok &&
        face->glyph->format == FT_GLYPH_FORMAT_OUTLINE;

    if ( !result )
    {
        metrics._xHeight = defaultXHeight;
        return std::optional<FontStorageBase::EMFontMetrics> { std::move ( metrics ) };
    }

    FT_Pos maxY = 0;
    FT_Outline const &outline = face->glyph->outline;

    for ( FT_Vector const &p : std::span<FT_Vector const> ( outline.points, static_cast<size_t> ( outline.n_points ) ) )
        maxY = std::max ( maxY, p.y );

    double const cases[] = { defaultXHeight, invEM * static_cast<double> ( maxY >> 6 ) };
    metrics._xHeight = cases[ static_cast<size_t> ( maxY > 0 ) ];

    return std::optional<FontStorageBase::EMFontMetrics> { std::move ( metrics ) };
}

} // namespace pbr


#endif // PBR_FONT_STORAGE_BASE_IPP
