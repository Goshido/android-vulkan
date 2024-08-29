#include <editor.hpp>
#include <logger.hpp>
#include <save_state.hpp>
#include <trace.hpp>


namespace editor {

namespace {

constexpr std::string_view CONFIG_PATH = R"__(%APPDATA%\Goshido Inc\Editor\editor.cfg)__";

constexpr std::string_view CONFIG_KEY_GPU = "user GPU";
constexpr std::string_view CONFIG_KEY_DPI = "DPI";

constexpr std::string_view CLI_USER_GPU = "--gpu";

} // end of anonumous namespace

Editor::Editor ( CommandLine &&commandLine ) noexcept:
    _commandLine ( std::move ( commandLine ) )
{
    // NOTHING
}

bool Editor::Run () noexcept
{
    AV_THREAD_NAME ( "Main" )

    if ( !InitModules () ) [[unlikely]]
        return false;

    EventLoop ();
    DestroyModules ();
    return true;
}

bool Editor::InitModules () noexcept
{
    AV_TRACE ( "Init modules" )

    if ( !_mainWindow.MakeWindow ( _messageQueue ) ) [[unlikely]]
        return false;

    Config const config = LoadConfig ();
    return _renderer.OnCreateDevice ( config._gpu, config._dpi );
}

void Editor::DestroyModules () noexcept
{
    AV_TRACE ( "Destroying modules" )

    if ( !_mainWindow.Destroy () )
        android_vulkan::LogError ( "Editor: Can't destroy main window" );

    SaveState config {};
    SaveState::Container &root = config.GetContainer ();
    root.Write ( CONFIG_KEY_GPU, _renderer.GetDeviceName () );
    root.Write ( CONFIG_KEY_DPI, _renderer.GetDPI () );

    _renderer.OnDestroyDevice ();

    if ( !config.Save ( CONFIG_PATH ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "Editor: Can't save config %s", CONFIG_PATH.data () );
    }
}

void Editor::EventLoop () noexcept
{
    ScheduleEventLoop ();

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = _messageQueue.Dequeue ();

        if ( message._type == eMessageType::CloseEditor )
            return;

        if ( message._type != eMessageType::RunEventLoop )
            _messageQueue.Enqueue ( std::move ( message ) );

        _mainWindow.Execute ();
        ScheduleEventLoop ();
    }
}

void Editor::ScheduleEventLoop () noexcept
{
    _messageQueue.Enqueue (
        Message
        {
            ._type = eMessageType::RunEventLoop,
            ._params = nullptr
        }
    );
}

std::string_view Editor::GetUserGPU () const noexcept
{
    size_t const count = _commandLine.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        if ( _commandLine[ i ] == CLI_USER_GPU )
        {
            return { _commandLine[ i + 1U ] };
        }
    }

    return {};
}

Editor::Config Editor::LoadConfig () noexcept
{
    AV_TRACE ( "Editor config load" )
    SaveState config {};

    if ( !config.Load ( CONFIG_PATH, true ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "Editor: Can't load config %s", CONFIG_PATH.data () );
        return {};
    }

    SaveState::Container const &root = config.GetContainer ();
    Config result {};

    if ( std::string_view const gpu = GetUserGPU (); !gpu.empty () ) [[unlikely]]
        result._gpu = gpu;
    else
        result._gpu = root.Read ( CONFIG_KEY_GPU, DEFAULT_GPU );

    result._dpi = root.Read ( CONFIG_KEY_DPI, DEFAULT_DPI );
    return result;
}

} // namespace editor
