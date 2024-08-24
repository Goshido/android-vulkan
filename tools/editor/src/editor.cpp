#include <crash_dump.hpp>
#include <editor.hpp>
#include <logger.hpp>
#include <save_state.hpp>
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

        SaveState saveState {};
        SaveState::Container &root = saveState.GetContainer ();

        using namespace std::literals;

        SaveState::Container &magicArray = root.WriteArray ( "magic array"sv );
        magicArray.Write ( "one"sv );
        magicArray.Write ( "two"sv );
        magicArray.Write ( "three"sv );
        magicArray.Write ( "FOCK"sv );

        SaveState::Container &system = root.WriteContainer ( "system"sv );
        SaveState::Container &resolution = system.WriteArray ( "resolution"sv );
        resolution.Write ( static_cast<uint32_t> ( 1920U ) );
        resolution.Write ( static_cast<uint32_t> ( 1080U ) );

        system.Write ( "mode"sv, "fullscreen"sv );
        system.Write ( "vSync"sv, true );

        if ( !saveState.Save ( R"__(%APPDATA%\Goshido Inc\Editor\editor.cfg)__"sv ) )
        {
            return false;
        }
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
