#include <av_assert.hpp>
#include <editor.hpp>
#include <logger.hpp>
#include <save_state.hpp>
#include <trace.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>

GX_RESTORE_WARNING_STATE


namespace editor {

namespace {

constexpr std::string_view CONFIG_PATH = R"__(%APPDATA%\Goshido Inc\Editor\editor.cfg)__";

constexpr std::string_view CONFIG_KEY_GPU = "user GPU";
constexpr std::string_view CONFIG_KEY_DPI = "DPI";
constexpr std::string_view CONFIG_KEY_VSYNC = "vSync";

constexpr std::string_view CLI_USER_GPU = "--gpu";

} // end of anonumous namespace

//----------------------------------------------------------------------------------------------------------------------

Editor::Editor ( CommandLine &&commandLine ) noexcept:
    _commandLine ( std::move ( commandLine ) )
{
    // NOTHING
}

bool Editor::Run () noexcept
{
    AV_THREAD_NAME ( "Main" )
    std::filesystem::current_path ( "../../assets" );

    if ( !InitModules () ) [[unlikely]]
        return false;

    EventLoop ();
    DestroyModules ();
    return true;
}

bool Editor::InitModules () noexcept
{
    AV_TRACE ( "Init modules" )
    Config const config = LoadConfig ();

    std::thread (
        [ this, &config ] () noexcept
        {
            AV_THREAD_NAME ( "Init Vulkan" )

            _messageQueue.EnqueueBack (
                Message
                {
                    ._type = eMessageType::VulkanInitReport,

                    ._params = reinterpret_cast<void*> (
                        static_cast<uintptr_t> ( _renderer.OnCreateDevice ( config._gpu, config._dpi ) )
                    )
                }
            );
        }
    ).detach ();

    bool result = _mainWindow.MakeWindow ( _messageQueue );

    for ( bool waiting = true; waiting; )
    {
        AV_TRACE ( "Editor: async init loop" )
        Message message = _messageQueue.DequeueBegin ();

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::VulkanInitReport:
                _messageQueue.DequeueEnd ();
                result &= static_cast<bool> ( reinterpret_cast<uintptr_t> ( message._params ) );
                waiting = false;
            break;

            case eMessageType::WindowVisibilityChanged:
                OnWindowVisibilityChanged ( std::move ( message ) );
            break;

            default:
                _messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }

    if ( !result ) [[unlikely]]
    {
        DestroyModules ();
        return false;
    }

    result = _renderer.OnCreateSwapchain (
        reinterpret_cast<android_vulkan::WindowHandle> ( _mainWindow.GetNativeWindow () ),
        config._vSync
    );

    if ( !result || !_renderSession.Init ( _messageQueue, _renderer ) ) [[unlikely]]
        return false;

    _runningModules = 1U;
    return true;
}

void Editor::DestroyModules () noexcept
{
    AV_TRACE ( "Destroying modules" )

    while ( _runningModules )
    {
        Message message = _messageQueue.DequeueBegin ();

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::FrameComplete:
                _messageQueue.DequeueEnd ();
            break;

            case eMessageType::ModuleStopped:
                OnModuleStopped ();
            break;

            default:
                _messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }

    if ( !_mainWindow.Destroy () ) [[unlikely]]
        android_vulkan::LogError ( "Editor: Can't destroy main window" );

    SaveState config {};
    SaveState::Container &root = config.GetContainer ();
    root.Write ( CONFIG_KEY_GPU, _renderer.GetDeviceName () );
    root.Write ( CONFIG_KEY_DPI, _renderer.GetDPI () );
    root.Write ( CONFIG_KEY_VSYNC, _renderer.GetVSync () );

    if ( !config.Save ( CONFIG_PATH ) ) [[unlikely]]
        android_vulkan::LogError ( "Editor: Can't save config %s", CONFIG_PATH.data () );

    _renderSession.Destroy ();
    _renderer.OnDestroySwapchain ( false );
    _renderer.OnDestroyDevice ();
}

void Editor::EventLoop () noexcept
{
    ScheduleEventLoop ();

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = _messageQueue.DequeueBegin ();

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::CloseEditor:
                OnShutdown ();
            return;

            case eMessageType::FrameComplete:
                OnFrameComplete ();
            break;

            case eMessageType::ModuleStopped:
                OnModuleStopped ();
            break;

            case eMessageType::RecreateSwapchain:
                OnRecreateSwapchain ();
            break;

            case eMessageType::RunEventLoop:
                OnRunEvent ();
            break;

            case eMessageType::WindowVisibilityChanged:
                OnWindowVisibilityChanged ( std::move ( message ) );
            break;

            default:
                _messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

void Editor::OnFrameComplete () noexcept
{
    _messageQueue.DequeueEnd ();
    _frameComplete = true;
}

void Editor::OnModuleStopped () noexcept
{
    _messageQueue.DequeueEnd ();
    --_runningModules;
}

void Editor::OnRecreateSwapchain () noexcept
{
    AV_TRACE ( "Recreate swapchain" )
    _messageQueue.DequeueEnd ();

    bool result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( _renderer.GetQueue () ),
        "editor::Editor::OnRecreateSwapchain",
        "Can't wait queue idle"
    );

    if ( !result ) [[unlikely]]
    {
        // FUCK
        AV_ASSERT ( false )
        return;
    }

    _renderer.OnDestroySwapchain ( true );

    result = _renderer.OnCreateSwapchain (
        reinterpret_cast<android_vulkan::WindowHandle> ( _mainWindow.GetNativeWindow () ),
        _renderer.GetVSync ()
    );

    if ( !result ) [[unlikely]]
    {
        // FUCK
        AV_ASSERT ( false )
        return;
    }

    _messageQueue.EnqueueBack (
        Message
        {
            ._type = eMessageType::SwapchainCreated,
            ._params = nullptr
        }
    );
}

void Editor::OnRunEvent () noexcept
{
    _messageQueue.DequeueEnd ();
    _mainWindow.Execute ();

    if ( !_stopRendering & _frameComplete ) [[likely]]
    {
        _messageQueue.EnqueueBack (
            Message
            {
                ._type = eMessageType::RenderFrame,
                ._params = nullptr
            }
        );

        _frameComplete = false;
    }

    ScheduleEventLoop ();
}

void Editor::OnShutdown () noexcept
{
    AV_TRACE ( "Shutdown" )
    _messageQueue.DequeueEnd ();

    _messageQueue.EnqueueBack (
        Message
        {
            ._type = eMessageType::Shutdown,
            ._params = nullptr
        }
    );

    // At this point only last eMessageType::RunEventLoop left in message queue. Need to dequeue it.
    for ( ; ; )
    {
        Message message = _messageQueue.DequeueBegin ();

        if ( message._type == eMessageType::RunEventLoop )
        {
            _messageQueue.DequeueEnd ();
            return;
        }

        _messageQueue.DequeueEnd ( std::move ( message ) );
    }
}

void Editor::OnWindowVisibilityChanged ( Message &&message ) noexcept
{
    _messageQueue.DequeueEnd ();
    _stopRendering = static_cast<bool> ( reinterpret_cast<uintptr_t> ( message._params ) );
}

void Editor::ScheduleEventLoop () noexcept
{
    _messageQueue.EnqueueBack (
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
    AV_TRACE ( "Editor: load config" )
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
    result._vSync = root.Read ( CONFIG_KEY_VSYNC, DEFAULT_VSYNC );
    return result;
}

} // namespace editor
