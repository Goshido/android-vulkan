#include <crash_dump.hpp>
#include <editor.hpp>
#include <logger.hpp>
#include <trace.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <chrono>
#include <thread>

GX_RESTORE_WARNING_STATE


namespace editor {

Editor::Editor ( CommandLine &&/*args*/ ) noexcept
{
    // NOTHING
}

bool Editor::Run () noexcept
{
    if ( !CrashDump::Install () ) [[unlikely]]
        return false;

    {
        AV_TRACE ( "Initialization modules" )
        std::this_thread::sleep_for ( std::chrono::milliseconds ( 7U ) );
    }

    while ( _run )
    {
        AV_TRACE ( "Event loop" )
        PumpEvents ();
        ExecuteEvents ();

        std::this_thread::sleep_for ( std::chrono::milliseconds ( 15U ) );
        // FUCK
    }

    return true;
}

void Editor::PumpEvents () noexcept
{
    AV_TRACE ( "Pump events" )
    std::this_thread::sleep_for ( std::chrono::milliseconds ( 10U ) );
}

void Editor::ExecuteEvents () noexcept
{
    AV_TRACE ( "Execute events" )
    std::this_thread::sleep_for ( std::chrono::milliseconds ( 42U ) );
}

} // namespace editor
