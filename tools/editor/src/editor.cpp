#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <editor.hpp>
#include <logger.hpp>
#include <save_state.hpp>
#include <trace.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>


namespace editor {

namespace {

constexpr std::string_view CONFIG_PATH = R"__(%APPDATA%\Goshido Inc\Editor\editor.cfg)__";

constexpr std::string_view CONFIG_KEY_GPU = "user GPU";
constexpr std::string_view CONFIG_KEY_UI_ZOOM = "UI zoom";
constexpr std::string_view CONFIG_KEY_VSYNC = "vSync";

constexpr std::string_view CLI_USER_GPU = "--gpu";

// It prevents buzy loop.
// [2024/09/22] It's impossible to sleep less than 1 ms on Windows.
constexpr std::chrono::milliseconds IDLE ( 1U );

constexpr float COMFORTABLE_VIEW_DISTANCE_METERS = 8.0e-1F;

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
    _uiZoom = config._uiZoom;

    std::thread (
        [ this, &config ] () noexcept
        {
            AV_THREAD_NAME ( "Init Vulkan" )

            _messageQueue.EnqueueBack (
                Message
                {
                    ._type = eMessageType::VulkanInitReport,

                    ._params = reinterpret_cast<void*> (
                        static_cast<uintptr_t> (
                            _renderer.OnCreateDevice ( config._gpu )
                        )
                    ),

                    ._serialNumber = 0U
                }
            );
        }
    ).detach ();

    bool result = _mainWindow.MakeWindow ( _messageQueue );
    std::optional<uint32_t> knownSerialNumber {};

    for ( bool waiting = true; waiting; )
    {
        AV_TRACE ( "Editor: async init loop" )
        Message message = _messageQueue.DequeueBegin ( knownSerialNumber );

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
                knownSerialNumber = message._serialNumber;
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

    float const dpi = _uiZoom * _mainWindow.GetDPI ();
    _renderer.OnSetDPI ( dpi );
    pbr::CSSUnitToDevicePixel::Init ( dpi, COMFORTABLE_VIEW_DISTANCE_METERS );

    result = _renderer.OnCreateSwapchain (
        reinterpret_cast<android_vulkan::WindowHandle> ( _mainWindow.GetNativeWindow () ),
        config._vSync
    );

    if ( !result ) [[unlikely]]
        return false;

    _uiManager.Init ( _messageQueue );
    _runningModules = 1U;

    if ( !result || !_renderSession.Init ( _messageQueue, _renderer, _uiManager ) ) [[unlikely]]
        return false;

    ++_runningModules;
    return true;
}

void Editor::DestroyModules () noexcept
{
    AV_TRACE ( "Destroying modules" )
    std::optional<uint32_t> knownSerialNumber {};

    while ( _runningModules )
    {
        Message message = _messageQueue.DequeueBegin ( knownSerialNumber );

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
                knownSerialNumber = message._serialNumber;
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
    root.Write ( CONFIG_KEY_UI_ZOOM, _uiZoom );
    root.Write ( CONFIG_KEY_VSYNC, _renderer.GetVSync () );

    if ( !config.Save ( CONFIG_PATH ) ) [[unlikely]]
        android_vulkan::LogError ( "Editor: Can't save config %s", CONFIG_PATH.data () );

    _renderSession.Destroy ();
    _uiManager.Destroy ();
    _renderer.OnDestroySwapchain ( false );
    _renderer.OnDestroyDevice ();
}

void Editor::EventLoop () noexcept
{
    ScheduleEventLoop ();
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        std::this_thread::sleep_for ( IDLE );
        Message message = _messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::CloseEditor:
                OnShutdown ();
            return;

            case eMessageType::DPIChanged:
                OnDPIChanged ( std::move ( message ) );
            break;

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
                lastRefund = message._serialNumber;
                _messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

void Editor::OnDPIChanged ( Message &&message ) noexcept
{
    _messageQueue.DequeueEnd ();
    _renderer.OnSetDPI ( _uiZoom * static_cast<float> ( reinterpret_cast<uintptr_t> ( message._params ) ) );
    // FUCK
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
            ._params = nullptr,
            ._serialNumber = 0U
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
                ._params = nullptr,
                ._serialNumber = 0U
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
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );

    std::optional<Message::SerialNumber> lastRefund {};

    while ( _runningModules )
    {
        std::this_thread::sleep_for ( IDLE );
        Message message = _messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::FrameComplete:
                OnFrameComplete ();
            break;

            case eMessageType::ModuleStopped:
                OnModuleStopped ();
            break;

            default:
                lastRefund = message._serialNumber;
                _messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
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
            ._params = nullptr,
            ._serialNumber = 0U
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

    result._vSync = root.Read ( CONFIG_KEY_VSYNC, DEFAULT_VSYNC );
    result._uiZoom = root.Read ( CONFIG_KEY_UI_ZOOM, DEFAULT_UI_ZOOM );
    return result;
}

} // namespace editor
