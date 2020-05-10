#include <file.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <android/asset_manager.h>

GX_RESTORE_WARNING_STATE

#include <logger.h>


namespace android_vulkan {

extern AAssetManager* g_AssetManager;

File::File ( std::string &filePath ):
    _filePath ( filePath )
{
    // NOTHING
}

File::File ( std::string &&filePath ):
    _filePath ( std::move ( filePath ) )
{
    // NOTHING
}

const std::vector<uint8_t>& File::GetContent () const
{
    return _content;
}

bool File::IsContentLoaded () const
{
    return !_content.empty ();
}

bool File::LoadContent ()
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
        assert ( !"File::LoadContent - Can't open file." );
        return false;
    }

    const auto size = static_cast<const size_t> ( AAsset_getLength ( asset ) );

    if ( !size )
    {
        LogWarning ( "File::LoadContent - File %s is empty!", _filePath.c_str () );
        AAsset_close ( asset );
        _content.clear ();
        return true;
    }

    _content.resize ( size );
    const auto readBytes = static_cast<const size_t> ( AAsset_read ( asset, _content.data (), size ) );
    AAsset_close ( asset );

    if ( size == readBytes )
        return true;

    LogError ( "File::LoadContent - Can't load whole file content %s.", _filePath.c_str () );
    assert ( !"File::LoadContent - Can't load whole file content." );
    return false;
}

} // namespace android_vulkan
