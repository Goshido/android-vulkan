#include <pbr/font_storage.h>
#include <file.h>
#include <logger.h>


namespace pbr {

[[maybe_unused]] bool FontStorage::Atlas::AddLayer ( android_vulkan::Renderer &/*renderer*/,
    VkCommandBuffer /*commandBuffer*/
) noexcept
{
    // TODO
    return true;
}

void FontStorage::Atlas::Destroy () noexcept
{
    // TODO
}

void FontStorage::Atlas::SetResolution ( VkExtent2D const &resolution ) noexcept
{
    _resolution = resolution;
}

//----------------------------------------------------------------------------------------------------------------------

bool FontStorage::Init ( VkExtent2D const &nativeViewport ) noexcept
{
    _atlas.SetResolution ( nativeViewport );
    return CheckFTResult ( FT_Init_FreeType ( &_library ), "pbr::FontStorage::Init", "Can't init FreeType" );
}

void FontStorage::Destroy ( android_vulkan::Renderer &/*renderer*/ ) noexcept
{
    _atlas.Destroy ();

    if ( !_library )
        return;

    bool const result = CheckFTResult ( FT_Done_FreeType ( _library ),
        "pbr::FontStorage::Destroy",
        "Can't close FreeType"
    );

    if ( result )
    {
        _library = nullptr;
    }
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
            ._glyphs = {}
        }
    );

    return status.first;
}

FontStorage::GlyphInfo const& FontStorage::GetGlyphInfo ( android_vulkan::Renderer &/*renderer*/,
    Font /*font*/,
    char32_t /*character*/
) noexcept
{
    //FontData& fontData = _fonts[ fontHash ];

    // TODO
    constexpr static GlyphInfo dummy {};
    return dummy;
}

bool FontStorage::CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept
{
    if ( result == FT_Err_Ok )
        return true;

    android_vulkan::LogError ( "%s - %s. Error: %s.", from, message, FT_Error_String ( result ) );
    return false;
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

} // namespace pbr
