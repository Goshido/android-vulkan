#include <pbr/font_storage.h>
#include <logger.h>


namespace pbr {

bool FontStorage::Init () noexcept
{
    return CheckFTResult ( FT_Init_FreeType ( &_library ), "pbr::FontStorage::Init", "Can't init FreeType" );
}

void FontStorage::Destroy () noexcept
{
    if ( !_library )
        return;

    [[maybe_unused]] bool const result = CheckFTResult ( FT_Done_FreeType ( _library ),
        "pbr::FontStorage::Destroy",
        "Can't close FreeType"
    );
}

FontStorage::GlyphInfo const& FontStorage::GetGlyphInfo ( android_vulkan::Renderer &/*renderer*/,
    std::string const &/*font*/,
    uint32_t /*size*/,
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

} // namespace pbr
