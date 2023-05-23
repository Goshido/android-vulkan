#include <file.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>
#include <fstream>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

File::File ( std::string &&filePath ) noexcept:
    _filePath ( std::move ( filePath ) )
{
    // NOTHING
}

File::File ( std::string_view const &filePath ) noexcept:
    _filePath ( filePath )
{
    // NOTHING
}

File::File ( char const* filePath ) noexcept:
    _filePath ( filePath )
{
    // NOTHING
}

std::vector<uint8_t>& File::GetContent () noexcept
{
    return _content;
}

[[maybe_unused]] std::vector<uint8_t> const& File::GetContent () const noexcept
{
    return _content;
}

[[maybe_unused]] std::string& File::GetPath () noexcept
{
    return _filePath;
}

[[maybe_unused]] std::string const& File::GetPath () const noexcept
{
    return _filePath;
}

bool File::IsContentLoaded () const noexcept
{
    return !_content.empty ();
}

bool File::IsExist () const noexcept
{
    return std::filesystem::exists ( _filePath );
}

bool File::LoadContent () noexcept
{
    if ( IsContentLoaded () )
    {
        LogWarning ( "File::LoadContent - File can be loaded only once [%s].", _filePath.c_str () );
        return true;
    }

    std::ifstream stream ( _filePath, std::ios::binary | std::ios::ate );

    if ( !stream.is_open () )
    {
        LogError ( "File::LoadContent - Can't open file %s.", _filePath.c_str () );
        return false;
    }

    auto const size = stream.tellg ();

    if ( !size )
    {
        LogWarning ( "File::LoadContent - File %s is empty!", _filePath.c_str () );
        _content.clear ();
        return true;
    }

    stream.seekg ( 0, std::ios::beg );
    _content.resize ( static_cast<size_t> ( size ) );

    if ( stream.read ( reinterpret_cast<char *> ( _content.data () ), size ).tellg () == size )
        return true;

    LogError ( "File::LoadContent - Can't load whole file content %s.", _filePath.c_str () );
    return false;
}

} // namespace android_vulkan
