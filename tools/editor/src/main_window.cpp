#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <logger.hpp>
#include <main_window.hpp>
#include <mouse_key_event.hpp>
#include <mouse_move_event.hpp>
#include <os_utils.hpp>
#include <save_state.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace editor {

namespace {

enum class eWindowState : uint8_t
{
    Normal = 0U,
    Minimized = 1U,
    Maximized = 2U
};

constexpr std::string_view CONFIG_PATH = R"__(%APPDATA%\Goshido Inc\Editor\window.cfg)__";

constexpr std::string_view CONFIG_KEY_STATE = "state";
constexpr eWindowState DEFAULT_STATE = eWindowState::Normal;

constexpr std::string_view CONFIG_KEY_POSITION = "position";
constexpr int32_t DEFAULT_X = 100;
constexpr int32_t DEFAULT_Y = 100;

constexpr std::string_view CONFIG_KEY_SIZE = "size";
constexpr uint16_t DEFAULT_WIDTH = 640U;
constexpr uint16_t DEFAULT_HEIGHT = 480U;

constexpr POINT MINIMUM_WINDOW_SIZE
{
    .x = 242,
    .y = 175
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool MainWindow::MakeWindow ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Making OS window" )

    _messageQueue = &messageQueue;
    HMODULE const module = GetModuleHandleA ( nullptr );
    constexpr int exeIconResourceID = 1;

    CreateCursors ();

    WNDCLASSA const wndClass
    {
        .style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = &MainWindow::WindowHandler,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = module,
        .hIcon = LoadIconA ( module, MAKEINTRESOURCEA ( exeIconResourceID ) ),
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

    _messageQueue = nullptr;

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

void MainWindow::CaptureMouse () noexcept
{
    SetCapture ( _hwnd );
}

void MainWindow::ReleaseMouse () noexcept
{
    ReleaseCapture ();
}

void MainWindow::ChangeCursor ( eCursor cursor ) noexcept
{
    SetCursor ( _cursors.find ( cursor )->second );
}

float MainWindow::GetDPI () const noexcept
{
    return static_cast<float> ( GetDpiForWindow ( _hwnd ) );
}

HWND MainWindow::GetNativeWindow () const noexcept
{
    return _hwnd;
}

void MainWindow::OnClose () noexcept
{
    AV_TRACE ( "Main window: close" )

    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::CloseEditor,
            ._params = nullptr,
            ._serialNumber = 0U
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

void MainWindow::OnDPIChanged ( WPARAM wParam, LPARAM lParam ) noexcept
{
    AV_TRACE ( "Main window: DPI changed" )

    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::DPIChanged,
            ._params = reinterpret_cast<void*> ( static_cast<uintptr_t> ( LOWORD ( wParam ) ) ),
            ._serialNumber = 0U
        }
    );

    RECT const &rect = *reinterpret_cast<RECT const*> ( lParam );

    MoveWindow ( _hwnd,
        static_cast<int> ( rect.left ),
        static_cast<int> ( rect.right ),
        static_cast<int> ( rect.right - rect.left ),
        static_cast<int> ( rect.bottom - rect.top ),
        TRUE
    );
}

void MainWindow::OnGetMinMaxInfo ( LPARAM lParam ) noexcept
{
    reinterpret_cast<MINMAXINFO*> ( lParam )->ptMinTrackSize = MINIMUM_WINDOW_SIZE;
}

void MainWindow::OnLButtonDown ( LPARAM lParam ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::MouseKeyDown,

            ._params = new MouseKeyEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._key = eKey::LeftMouseButton
            },

            ._serialNumber = 0U
        }
    );
}

void MainWindow::OnLButtonUp ( LPARAM lParam ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::MouseKeyUp,

            ._params = new MouseKeyEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._key = eKey::LeftMouseButton
            },

            ._serialNumber = 0U
        }
    );
}

void MainWindow::OnMButtonDown ( LPARAM lParam ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::MouseKeyDown,

            ._params = new MouseKeyEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._key = eKey::MiddleMouseButton
            },

            ._serialNumber = 0U
        }
    );
}

void MainWindow::OnMButtonUp ( LPARAM lParam ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::MouseKeyUp,

            ._params = new MouseKeyEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._key = eKey::MiddleMouseButton
            },

            ._serialNumber = 0U
        }
    );
}

void MainWindow::OnMouseMove ( LPARAM lParam ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::MouseMoved,

            ._params = new MouseMoveEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._eventID = ++_mouseMoveEventID
            },

            ._serialNumber = 0U
        }
    );
}

void MainWindow::OnRButtonDown ( LPARAM lParam ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::MouseKeyDown,

            ._params = new MouseKeyEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._key = eKey::RightMouseButton
            },

            ._serialNumber = 0U
        }
    );
}

void MainWindow::OnRButtonUp ( LPARAM lParam ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::MouseKeyUp,

            ._params = new MouseKeyEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._key = eKey::RightMouseButton
            },

            ._serialNumber = 0U
        }
    );
}

void MainWindow::OnSize ( WPARAM wParam ) noexcept
{
    AV_TRACE ( "Main window: size" )

    _messageQueue->EnqueueBack (
        {
            ._type = eMessageType::WindowVisibilityChanged,
            ._params = reinterpret_cast<void*> ( static_cast<uintptr_t> ( wParam == SIZE_MINIMIZED ) ),
            ._serialNumber = 0U
        }
    );
}

void MainWindow::CreateCursors () noexcept
{
    AV_TRACE ( "Main window: create cursors" )

    constexpr auto flags = static_cast<UINT> ( AV_VK_FLAG ( LR_SHARED ) | AV_VK_FLAG ( LR_DEFAULTSIZE ) );

    _cursors.insert (
        std::make_pair ( eCursor::Arrow,
            static_cast<HCURSOR> ( LoadImageW ( nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, flags ) )
        )
    );

    _cursors.insert (
        std::make_pair ( eCursor::NorthSouth,
            static_cast<HCURSOR> ( LoadImageW ( nullptr, IDC_SIZENS, IMAGE_CURSOR, 0, 0, flags ) )
        )
    );

    _cursors.insert (
        std::make_pair ( eCursor::WestEast,
            static_cast<HCURSOR> ( LoadImageW ( nullptr, IDC_SIZEWE, IMAGE_CURSOR, 0, 0, flags ) )
        )
    );

    _cursors.insert (
        std::make_pair ( eCursor::Cross,
            static_cast<HCURSOR> ( LoadImageW ( nullptr, IDC_SIZEALL, IMAGE_CURSOR, 0, 0, flags ) )
        )
    );

    _cursors.insert (
        std::make_pair ( eCursor::NorthWestSouthEast,
            static_cast<HCURSOR> ( LoadImageW ( nullptr, IDC_SIZENWSE, IMAGE_CURSOR, 0, 0, flags ) )
        )
    );

    _cursors.insert (
        std::make_pair ( eCursor::NorthEastSouthWest,
            static_cast<HCURSOR> ( LoadImageW ( nullptr, IDC_SIZENESW, IMAGE_CURSOR, 0, 0, flags ) )
        )
    );
}

void MainWindow::Save () noexcept
{
    AV_TRACE ( "Main window: save state" )

    WINDOWPLACEMENT placement
    {
        .length = static_cast<UINT> ( sizeof ( WINDOWPLACEMENT ) ),
        .showCmd = SW_HIDE,
        .ptMinPosition {},
        .ptMaxPosition {},
        .rcNormalPosition {}
    };

    GetWindowPlacement ( _hwnd, &placement );
    eWindowState state;

    GX_DISABLE_WARNING ( 4061 )

    switch ( placement.showCmd )
    {
        case SW_SHOWMINIMIZED:
            state = eWindowState::Minimized;
        break;

        case SW_SHOWMAXIMIZED:
            state = eWindowState::Maximized;
        break;

        case SW_SHOWNORMAL:
            [[fallthrough]];

        default:
            state = eWindowState::Normal;
        break;
    }

    GX_ENABLE_WARNING ( 4061 )

    SaveState config {};
    SaveState::Container &root = config.GetContainer ();

    // Having main window in minimized state after starting editor is weird. If window is in minimized state right now
    // than we move it into normal state.
    root.Write ( CONFIG_KEY_STATE,
        static_cast<uint8_t> ( state == eWindowState::Minimized ? eWindowState::Normal : state )
    );

    // [2024/08/31, Windows 11 Pro, 23H2, 22631.4112]
    // Issue with maximize. If window is maximized than x and y coordinates are incorrect. It's because
    // OS sends WM_MOVE with x = 2, y = 25 and only than OS sends WM_SIZE. In order to have correct normal
    // windows coordinates we need to translate window back to normal state and receive new x and y coordinates.
    // We can't use GetWindowPlacement because it returns coordinates in workspace coordinates and we need
    // screen coordinates. Ignoring coordinate conventions will fuck up users who places taskbar in top or
    // left side of the screen.
    //
    // Another issue with minimize. Moving to minimized state sends WM_MOVE with x = -32000, y = -32000.
    // But moving from minimized state to normal state does not sends anything. So it's needed to explicitly request
    // window position back.
    if ( state != eWindowState::Normal )
        ShowWindow ( _hwnd, SW_NORMAL );

    RECT rect;
    GetWindowRect ( _hwnd, &rect );

    SaveState::Container &position = root.WriteArray ( CONFIG_KEY_POSITION );
    position.Write ( static_cast<int32_t> ( rect.left ) );
    position.Write ( static_cast<int32_t> ( rect.top ) );

    SaveState::Container &size = root.WriteArray ( CONFIG_KEY_SIZE );
    size.Write ( static_cast<uint16_t> ( rect.right - rect.left ) );
    size.Write ( static_cast<uint16_t> ( rect.bottom - rect.top ) );

    if ( !config.Save ( CONFIG_PATH ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "MainWindow: Can't save config %s", CONFIG_PATH.data () );
    }
}

void MainWindow::Load () noexcept
{
    AV_TRACE ( "Main window: load state" )
    SaveState config {};

    if ( !config.Load ( CONFIG_PATH, true ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "MainWindow: Can't load config %s", CONFIG_PATH.data () );
        return;
    }

    SaveState::Container const &root = config.GetContainer ();

    SaveState::Container const &position = root.ReadArray ( CONFIG_KEY_POSITION );
    auto const x = static_cast<int> ( position.Read ( DEFAULT_X ) );
    auto const y = static_cast<int> ( position.Read ( DEFAULT_Y ) );

    SaveState::Container const &size = root.ReadArray ( CONFIG_KEY_SIZE );
    auto const w = static_cast<int> ( size.Read ( DEFAULT_WIDTH ) );
    auto const h = static_cast<int> ( size.Read ( DEFAULT_HEIGHT ) );

    MoveWindow ( _hwnd, x, y, w, h, FALSE );

    switch ( static_cast<eWindowState> ( root.Read ( CONFIG_KEY_STATE, static_cast<uint8_t> ( DEFAULT_STATE ) ) ) )
    {
        case eWindowState::Minimized:
            ShowWindow ( _hwnd, SW_MINIMIZE );
        break;

        case eWindowState::Maximized:
            ShowWindow ( _hwnd, SW_MAXIMIZE );
        break;

        case eWindowState::Normal:
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

        case WM_DPICHANGED:
            mainWindow.OnDPIChanged ( wParam, lParam );
        break;

        case WM_GETMINMAXINFO:
            mainWindow.OnGetMinMaxInfo ( lParam );
        return 0;

        case WM_LBUTTONDOWN:
            mainWindow.OnLButtonDown ( lParam );
        return 0;

        case WM_LBUTTONUP:
            mainWindow.OnLButtonUp ( lParam );
        return 0;

        case WM_MBUTTONDOWN:
            mainWindow.OnMButtonDown ( lParam );
        return 0;

        case WM_MBUTTONUP:
            mainWindow.OnMButtonUp ( lParam );
        return 0;

        case WM_MOUSEMOVE:
            mainWindow.OnMouseMove ( lParam );
        break;

        case WM_RBUTTONDOWN:
            mainWindow.OnRButtonDown ( lParam );
        return 0;

        case WM_RBUTTONUP:
            mainWindow.OnRButtonUp ( lParam );
        return 0;

        case WM_SIZE:
            mainWindow.OnSize ( wParam );
        break;

        default:
            // NOTHING
        break;
    }

    return DefWindowProcA ( hwnd, msg, wParam, lParam );
}

} // namespace editor
