#include <crash_dump.hpp>
#include <editor.hpp>
#include <logger.hpp>


namespace editor {

Editor::Editor ( CommandLine &&/*args*/ ) noexcept
{
    // TODO
}

bool Editor::Run () noexcept
{
    if ( !CrashDump::Install () ) [[unlikely]]
        return false;

    while ( _run )
    {
        // FUCK
    }

    return true;
}

} // namespace editor
