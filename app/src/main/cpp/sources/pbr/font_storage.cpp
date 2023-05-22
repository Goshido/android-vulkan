#include <pbr/font_storage.h>
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

FontStorage::GlyphInfo const& FontStorage::GetGlyphInfo ( android_vulkan::Renderer &/*renderer*/,
    size_t /*fontHash*/,
    char32_t /*character*/
) noexcept
{
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

FontStorage::FontHash FontStorage::GetFontHash ( std::string_view font, uint32_t size ) noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine.
    constexpr size_t magic = 0x9E3779B9U;

    static std::hash<uint32_t> const hashInteger {};
    static std::hash<std::string_view> const hashStringView {};

    size_t hash = 0U;
    hash ^= hashStringView ( font ) + magic + ( hash << 6U ) + ( hash >> 2U );
    return hash ^ ( hashInteger ( size ) + magic + ( hash << 6U ) + ( hash >> 2U ) );
}

} // namespace pbr
