#include <av_assert.hpp>
#include <os_utils.hpp>
#include <trace.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <Windows.h>

GX_RESTORE_WARNING_STATE


namespace editor {

std::filesystem::path OSUtils::ResolvePath ( std::string_view const &file ) noexcept
{
    AV_TRACE ( "Resolving path" )
    char const* f = file.data ();
    DWORD const len = ExpandEnvironmentStringsA ( f, nullptr, 0U );
    std::string path {};
    path.resize ( static_cast<size_t> ( len ) );
    ExpandEnvironmentStringsA ( f, path.data (), len );
    return std::filesystem::path ( std::move ( path ) );
}

std::string OSUtils::ToString ( std::filesystem::path const &/*path*/ ) noexcept
{
    // FUCK
    AV_ASSERT ( false )
    return {};
}

} // namespace editor
