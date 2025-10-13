#include <precompiled_headers.hpp>
#include <file.hpp>
#include <logger.hpp>


namespace android_vulkan {

namespace {

// Android NDK 25.2.9519653 and Android 11 do not allow to have '../' in the asset path.
// Such path could not be resolved in runtime and AAssetManager_open returns nullptr.
[[nodiscard]] std::string NormalizeAssetPath ( std::string &&asset ) noexcept
{
    constexpr char hackRoot = '/';
    constexpr size_t hackRootSize = 1U;
    std::string p = std::filesystem::weakly_canonical ( std::filesystem::path ( hackRoot + asset ) ).string ();

    if ( p.empty () )
        return asset;

    // Removing hacky root directory without reallocation.
    char* str = p.data ();
    size_t const newSize = p.size () - hackRootSize;

    std::memmove ( str, str + hackRootSize, newSize );
    p.resize ( newSize );

    return p;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

extern AAssetManager* g_AssetManager;

//----------------------------------------------------------------------------------------------------------------------

File::File ( std::string &&filePath ) noexcept:
    _filePath ( NormalizeAssetPath ( std::move ( filePath ) ) )
{
    // NOTHING
}

File::File ( std::string_view const &filePath ) noexcept:
    _filePath ( NormalizeAssetPath ( std::string ( filePath ) ) )
{
    // NOTHING
}

File::File ( char const* filePath ) noexcept:
    _filePath ( NormalizeAssetPath ( filePath ) )
{
    // NOTHING
}

std::vector<uint8_t> &File::GetContent () noexcept
{
    return _content;
}

std::vector<uint8_t> const &File::GetContent () const noexcept
{
    return _content;
}

std::string &File::GetPath () noexcept
{
    return _filePath;
}

std::string const &File::GetPath () const noexcept
{
    return _filePath;
}

bool File::IsContentLoaded () const noexcept
{
    return !_content.empty ();
}

bool File::IsExist () const noexcept
{
    AAsset* asset = AAssetManager_open ( g_AssetManager, _filePath.c_str (), AASSET_MODE_STREAMING );

    if ( !asset )
        return false;

    AAsset_close ( asset );
    return true;
}

bool File::LoadContent () noexcept
{
    if ( IsContentLoaded () )
    {
        LogWarning ( "File::LoadContent - File can be loaded only once [%s].", _filePath.c_str () );
        return true;
    }

    AAsset* asset = AAssetManager_open ( g_AssetManager, _filePath.c_str (), AASSET_MODE_BUFFER );

    if ( !asset )
    {
        LogError ( "File::LoadContent - Can't open file %s.", _filePath.c_str () );
        return false;
    }

    auto const size = static_cast<size_t> ( AAsset_getLength ( asset ) );

    if ( !size )
    {
        LogWarning ( "File::LoadContent - File %s is empty!", _filePath.c_str () );
        AAsset_close ( asset );
        _content.clear ();
        return true;
    }

    _content.resize ( size );
    auto const readBytes = static_cast<size_t> ( AAsset_read ( asset, _content.data (), size ) );
    AAsset_close ( asset );

    if ( size == readBytes )
        return true;

    LogError ( "File::LoadContent - Can't load whole file content %s.", _filePath.c_str () );
    return false;
}

} // namespace android_vulkan
