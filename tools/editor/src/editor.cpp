#include <editor.hpp>
#include <logger.hpp>
#include <save_state.hpp>
#include <trace.hpp>


namespace editor {

namespace {

constexpr std::string_view CONFIG_PATH = R"__(%APPDATA%\Goshido Inc\Editor\editor.cfg)__";

constexpr std::string_view CONFIG_GPU_KEY = "user GPU";
constexpr std::string_view CONFIG_GPU_DEFAULT {};

constexpr std::string_view CONFIG_DPI_KEY = "DPI";
constexpr float CONFIG_DPI_DEFAULT = 96.0F;

constexpr std::string_view CLI_USER_GPU = "--gpu";

} // end of anonumous namespace

Editor::Editor ( CommandLine &&args ) noexcept
{
    size_t const count = args.size ();

    for ( size_t i = 1U; i < count; ++i )
    {
        char const* const key = args[ i ];

        if ( key != CLI_USER_GPU )
            continue;

        _gpu = args[ i + 1U ];
        break;
    }
}

bool Editor::Run () noexcept
{
    AV_THREAD_NAME ( "Main" )

    {
        AV_TRACE ( "Initialization modules" )

        if ( !_mainWindow.MakeWindow () ) [[unlikely]]
            return false;

        LoadConfig ();

        if ( !_renderer.OnCreateDevice ( _gpu, _dpi ) )
        {
            return false;
        }
    }

    while ( _run )
    {
        AV_TRACE ( "Event loop" )
        _mainWindow.Execute ();
    }

    {
        AV_TRACE ( "Destroying modules" )
        SaveConfig ();
    }

    return true;
}

void Editor::LoadConfig () noexcept
{
    AV_TRACE ( "Editor config load" )
    SaveState config {};

    if ( !config.Load ( CONFIG_PATH, true ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "Editor: Can't load config %s", CONFIG_PATH.data () );
        return;
    }

    SaveState::Container const &root = config.GetContainer ();

    if ( _gpu.empty () ) [[likely]]
        _gpu = root.Read ( CONFIG_GPU_KEY, CONFIG_GPU_DEFAULT );

    _dpi = root.Read ( CONFIG_DPI_KEY, CONFIG_DPI_DEFAULT );
}

void Editor::SaveConfig () noexcept
{
    AV_TRACE ( "Editor config save" )
    SaveState config {};

    SaveState::Container &root = config.GetContainer ();
    root.Write ( CONFIG_GPU_KEY, std::string_view ( _gpu ) );
    root.Write ( CONFIG_DPI_KEY, _dpi );

    if ( !config.Save ( CONFIG_PATH ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "Editor: Can't save config %s", CONFIG_PATH.data () );
    }
}

} // namespace editor
