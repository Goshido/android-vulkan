#include <crash_dump.hpp>
#include <editor.hpp>
#include <trace.hpp>


namespace editor {

Editor::Editor ( CommandLine &&/*args*/ ) noexcept
{
    // NOTHING
}

bool Editor::Run () noexcept
{
    if ( !CrashDump::Install () ) [[unlikely]]
        return false;

    AV_THREAD_NAME ( "Main" )

    {
        AV_TRACE ( "Initialization modules" )

        if ( !_mainWindow.MakeWindow () ) [[unlikely]]
        {
            return false;
        }
    }

    while ( _run )
    {
        AV_TRACE ( "Event loop" )
        _mainWindow.Execute ();
    }

    return true;
}

} // namespace editor
