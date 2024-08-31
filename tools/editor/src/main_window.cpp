#include <logger.hpp>
#include <main_window.hpp>
#include <os_utils.hpp>
#include <save_state.hpp>
#include <trace.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <tchar.h>
#include <strsafe.h>

GX_RESTORE_WARNING_STATE


namespace editor {

namespace {

constexpr std::string_view CONFIG_PATH = R"__(%APPDATA%\Goshido Inc\Editor\window.cfg)__";

constexpr std::string_view CONFIG_KEY_STATE = "state";
constexpr std::string_view CONFIG_KEY_POSITION = "position";
constexpr std::string_view CONFIG_KEY_SIZE = "size";

} // end of anonymous namespace

bool MainWindow::MakeWindow ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Making OS window" )
    _messageQueue = &messageQueue;
    HMODULE const module = GetModuleHandleA ( nullptr );

    WNDCLASSA const wndClass
    {
        .style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = &MainWindow::WindowHandler,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = module,
        .hIcon = LoadIconA ( module, MAKEINTRESOURCEA ( 1 ) ),
        .hCursor = nullptr,
        .hbrBackground = CreateSolidBrush ( RGB ( 115, 185, 0 ) ),
        .lpszMenuName = nullptr,
        .lpszClassName = "Android-Vulkan Editor-{8a3ef647-a264-4b1f-ae54-0d9856cf23e3}"
    };

    _classID = RegisterClassA ( &wndClass );

    if ( _classID == 0 ) [[unlikely]]
    {
        android_vulkan::LogError ( "MainWindow: Can't register class." );
        return false;
    }

    // Note '_hwnd' will be assigned in OnCreate method:
    // CreateWindowEx
    //     WM_CREATE
    //         OnCreate: -> HWND is assigned to '_hwnd'
    // control returns to caller code
    HWND const result = CreateWindowEx ( WS_EX_ACCEPTFILES | WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW,
        MAKEINTATOM ( _classID ),
        _T ( "Editor" ),
        WS_VISIBLE | WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
        100,
        100,
        640,
        480,
        nullptr,
        nullptr,
        module,
        this
    );

    if ( result ) [[likely]]
    {
        Load ();
        return true;
    }

    OSUtils::PrintLastError ( "MainWindow " );
    return false;
}

bool MainWindow::Destroy () noexcept
{
    AV_TRACE ( "Destroying main window" )

    if ( _hwnd ) [[likely]]
    {
        Save ();
        BOOL const result = DestroyWindow ( _hwnd );

        if ( result == 0 ) [[unlikely]]
        {
            OSUtils::PrintLastError ( "MainWindow" );
            return false;
        }

        Execute ();
        _hwnd = nullptr;
    }

    if ( _classID == 0 ) [[unlikely]]
        return true;

    if ( BOOL const result = UnregisterClass ( MAKEINTATOM ( _classID ), GetModuleHandle ( nullptr ) ); result != 0 )
    {
        [[likely]]
        return true;
    }

    android_vulkan::LogError ( "MainWindow: Can't unregister window class." );
    return false;
}

void MainWindow::Execute () noexcept
{
    AV_TRACE ( "Executing OS messages" )
    MSG msg {};

    while ( PeekMessageA ( &msg, _hwnd, 0U, 0U, PM_REMOVE ) )
    {
        DispatchMessageA ( &msg );
    }
}

HWND MainWindow::GetNativeWindow () const noexcept
{
    return _hwnd;
}

void MainWindow::OnClose () noexcept
{
    AV_TRACE ( "Main window: close" )

    _messageQueue->Enqueue (
        Message
        {
            ._type = eMessageType::CloseEditor,
            ._params = nullptr
        }
    );
}

void MainWindow::OnCreate ( HWND hwnd ) noexcept
{
    AV_TRACE ( "Main window: create" )
    _hwnd = hwnd;

    // To destinguish error it's needed to preset 0 as last error. Trick is described on MSDN:
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptra#return-value
    SetLastError ( 0U );
    LONG_PTR const result = SetWindowLongPtrA ( hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR> ( this ) );

    if ( ( result == 0 ) && ( GetLastError () == 0U ) ) [[likely]]
        return;

    android_vulkan::LogError ( "MainWindow: Can't connect MainWindow onject to HWND handle." );
    DestroyWindow ( hwnd );
}

void MainWindow::OnMove ( LPARAM lParam ) noexcept
{
    AV_TRACE ( "Main window: move" )

    if ( _state != eState::Normal )
        return;

    _x = static_cast<int32_t> ( static_cast<int16_t> ( ( LOWORD ( lParam ) ) ) );
    _y = static_cast<int32_t> ( static_cast<int16_t> ( ( HIWORD ( lParam ) ) ) );
}

void MainWindow::OnSize ( WPARAM wParam, LPARAM lParam ) noexcept
{
    AV_TRACE ( "Main window: size" )

    switch ( wParam )
    {
        case SIZE_MAXIMIZED:
            _state = eState::Maximized;
        break;

        case SIZE_MINIMIZED:
            _state = eState::Minimized;
        break;

        case SIZE_RESTORED:
            if ( _state == eState::Minimized )
            {
                // [2024/08/31, Windows 11 Pro, 23H2, 22631.4112] Moving to minimized state sends WM_MESSAGE with 
                // X = -32000, Y = -32000. But moving from minimized state normal state does not sends anything. So it's
                // needed to explicitly request window position back.
                RECT rect;
                GetWindowRect ( _hwnd, &rect );
                _x = static_cast<int32_t> ( rect.left );
                _y = static_cast<int32_t> ( rect.top );
            }

            _state = eState::Normal;
            _width = static_cast<uint16_t> ( LOWORD ( lParam ) );
            _height = static_cast<uint16_t> ( HIWORD ( lParam ) );
        break;

        default:
            // NOTHING
        return;
    }
}

void MainWindow::Save () noexcept
{
    AV_TRACE ( "Main window: Save state" )

    SaveState config {};
    SaveState::Container &root = config.GetContainer ();
    root.Write ( CONFIG_KEY_STATE, static_cast<uint8_t> ( _state == eState::Minimized ? eState::Normal : _state ) );

    SaveState::Container &size = root.WriteArray ( CONFIG_KEY_SIZE );
    size.Write ( _width );
    size.Write ( _height );

    if ( _state == eState::Maximized )
    {
        // [2024/08/31, Windows 11 Pro, 23H2, 22631.4112] If window is maximized than '_x' and '_y' coordinates are
        // incorrect. It's because Windows sends WM_MOVE with X = 2, Y = 25 and only than it sends WM_SIZE.
        // In order to have correct normal windows coordinates we need to translate window back to normal state
        // and receive new '_x' and '_y' coordinates. We can't use GetWindowPlacement because it returns coordinates
        // in workspace coordinates and we need screen coordinates. Ignoring coordinate conventions will fuck up 
        // users who places taskbar in top or left side of the screen. I also did not find WinAPI function to convert 
        // workspace coordinates to screen coordinates.
        ShowWindow ( _hwnd, SW_NORMAL );

        RECT rect;
        GetWindowRect ( _hwnd, &rect );
        _x = static_cast<int32_t> ( rect.left );
        _y = static_cast<int32_t> ( rect.top );
    }

    SaveState::Container &position = root.WriteArray ( CONFIG_KEY_POSITION );
    position.Write ( _x );
    position.Write ( _y );

    if ( !config.Save ( CONFIG_PATH ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "MainWindow: Can't save config %s", CONFIG_PATH.data () );
    }
}

void MainWindow::Load () noexcept
{
    AV_TRACE ( "Main window: Load state" )
    SaveState config {};

    if ( !config.Load ( CONFIG_PATH, true ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "MainWindow: Can't load config %s", CONFIG_PATH.data () );
        return;
    }

    SaveState::Container const &root = config.GetContainer ();

    SaveState::Container const &position = root.ReadArray ( CONFIG_KEY_POSITION );
    _x = position.Read ( DEFAULT_X );
    _y = position.Read ( DEFAULT_Y );

    SaveState::Container const &size = root.ReadArray ( CONFIG_KEY_SIZE );
    _width = size.Read ( DEFAULT_WIDTH );
    _height = size.Read ( DEFAULT_HEIGHT );

    MoveWindow ( _hwnd,
        static_cast<int> ( _x ),
        static_cast<int> ( _y ),
        static_cast<int> ( _width ),
        static_cast<int> ( _height ),
        FALSE
    );

    switch ( static_cast<eState> ( root.Read ( CONFIG_KEY_STATE, static_cast<uint8_t> ( DEFAULT_STATE ) ) ) )
    {
        case editor::MainWindow::eState::Minimized:
            ShowWindow ( _hwnd, SW_MINIMIZE );
        break;

        case editor::MainWindow::eState::Maximized:
            ShowWindow ( _hwnd, SW_MAXIMIZE );
        break;

        case editor::MainWindow::eState::Normal:
            [[fallthrough]];

        default:
            // NOTHING
        break;
    }
}

LRESULT CALLBACK MainWindow::WindowHandler ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    AV_TRACE ( "OS window message" )
    auto &mainWindow = *reinterpret_cast<MainWindow*> ( GetWindowLongPtrA ( hwnd, GWLP_USERDATA ) );

    switch ( msg )
    {
        case WM_CREATE:
        {
            auto const &createInfo = *reinterpret_cast<LPCREATESTRUCT> ( lParam );
            reinterpret_cast<MainWindow*> ( createInfo.lpCreateParams )->OnCreate ( hwnd );
            break;
        }

        case WM_CLOSE:
            mainWindow.OnClose ();
        return 0;

        case WM_MOVE:
            mainWindow.OnMove ( lParam );
        break;

        case WM_SIZE:
            mainWindow.OnSize ( wParam, lParam );
        break;

        default:
            // NOTHING
        break;
    }

    return DefWindowProcA ( hwnd, msg, wParam, lParam );
}

} // namespace editor
