#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <keyboard_key_event.hpp>
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

// See https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
constexpr WPARAM VK_KEY_0 = 0x30U;
constexpr WPARAM VK_KEY_1 = 0x31U;
constexpr WPARAM VK_KEY_2 = 0x32U;
constexpr WPARAM VK_KEY_3 = 0x33U;
constexpr WPARAM VK_KEY_4 = 0x34U;
constexpr WPARAM VK_KEY_5 = 0x35U;
constexpr WPARAM VK_KEY_6 = 0x36U;
constexpr WPARAM VK_KEY_7 = 0x37U;
constexpr WPARAM VK_KEY_8 = 0x38U;
constexpr WPARAM VK_KEY_9 = 0x39U;
constexpr WPARAM VK_KEY_A = 0x41U;
constexpr WPARAM VK_KEY_B = 0x42U;
constexpr WPARAM VK_KEY_C = 0x43U;
constexpr WPARAM VK_KEY_D = 0x44U;
constexpr WPARAM VK_KEY_E = 0x45U;
constexpr WPARAM VK_KEY_F = 0x46U;
constexpr WPARAM VK_KEY_G = 0x47U;
constexpr WPARAM VK_KEY_H = 0x48U;
constexpr WPARAM VK_KEY_I = 0x49U;
constexpr WPARAM VK_KEY_J = 0x4AU;
constexpr WPARAM VK_KEY_K = 0x4BU;
constexpr WPARAM VK_KEY_L = 0x4CU;
constexpr WPARAM VK_KEY_M = 0x4DU;
constexpr WPARAM VK_KEY_N = 0x4EU;
constexpr WPARAM VK_KEY_O = 0x4FU;
constexpr WPARAM VK_KEY_P = 0x50U;
constexpr WPARAM VK_KEY_Q = 0x51U;
constexpr WPARAM VK_KEY_R = 0x52U;
constexpr WPARAM VK_KEY_S = 0x53U;
constexpr WPARAM VK_KEY_T = 0x54U;
constexpr WPARAM VK_KEY_U = 0x55U;
constexpr WPARAM VK_KEY_V = 0x56U;
constexpr WPARAM VK_KEY_W = 0x57U;
constexpr WPARAM VK_KEY_X = 0x58U;
constexpr WPARAM VK_KEY_Y = 0x59U;
constexpr WPARAM VK_KEY_Z = 0x5AU;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow () noexcept
{
    _keyboardKeyMapper =
    {
        { VK_KEY_0, eKey::Key0 },
        { VK_KEY_1, eKey::Key1 },
        { VK_KEY_2, eKey::Key2 },
        { VK_KEY_3, eKey::Key3 },
        { VK_KEY_4, eKey::Key4 },
        { VK_KEY_5, eKey::Key5 },
        { VK_KEY_6, eKey::Key6 },
        { VK_KEY_7, eKey::Key7 },
        { VK_KEY_8, eKey::Key8 },
        { VK_KEY_9, eKey::Key9 },

        { VK_F1, eKey::KeyF1 },
        { VK_F2, eKey::KeyF2 },
        { VK_F3, eKey::KeyF3 },
        { VK_F4, eKey::KeyF4 },
        { VK_F5, eKey::KeyF5 },
        { VK_F6, eKey::KeyF6 },
        { VK_F7, eKey::KeyF7 },
        { VK_F8, eKey::KeyF8 },
        { VK_F9, eKey::KeyF9 },
        { VK_F10, eKey::KeyF10 },
        { VK_F11, eKey::KeyF11 },
        { VK_F12, eKey::KeyF12 },

        { VK_KEY_A, eKey::KeyA },
        { VK_KEY_B, eKey::KeyB },
        { VK_KEY_C, eKey::KeyC },
        { VK_KEY_D, eKey::KeyD },
        { VK_KEY_E, eKey::KeyE },
        { VK_KEY_F, eKey::KeyF },
        { VK_KEY_G, eKey::KeyG },
        { VK_KEY_H, eKey::KeyH },
        { VK_KEY_I, eKey::KeyI },
        { VK_KEY_J, eKey::KeyJ },
        { VK_KEY_K, eKey::KeyK },
        { VK_KEY_L, eKey::KeyL },
        { VK_KEY_M, eKey::KeyM },
        { VK_KEY_N, eKey::KeyN },
        { VK_KEY_O, eKey::KeyO },
        { VK_KEY_P, eKey::KeyP },
        { VK_KEY_Q, eKey::KeyQ },
        { VK_KEY_R, eKey::KeyR },
        { VK_KEY_S, eKey::KeyS },
        { VK_KEY_T, eKey::KeyT },
        { VK_KEY_U, eKey::KeyU },
        { VK_KEY_V, eKey::KeyV },
        { VK_KEY_W, eKey::KeyW },
        { VK_KEY_X, eKey::KeyX },
        { VK_KEY_Y, eKey::KeyY },
        { VK_KEY_Z, eKey::KeyZ },

        { VK_DOWN, eKey::KeyDown },
        { VK_LEFT, eKey::KeyLeft },
        { VK_RIGHT, eKey::KeyRight },
        { VK_UP, eKey::KeyUp },

        { VK_OEM_4, eKey::KeyLeftSquareBracket },
        { VK_OEM_6, eKey::KeyRightSquareBracket },

        { VK_NUMPAD0, eKey::KeyNumpad0 },
        { VK_NUMPAD1, eKey::KeyNumpad1 },
        { VK_NUMPAD2, eKey::KeyNumpad2 },
        { VK_NUMPAD3, eKey::KeyNumpad3 },
        { VK_NUMPAD4, eKey::KeyNumpad4 },
        { VK_NUMPAD5, eKey::KeyNumpad5 },
        { VK_NUMPAD6, eKey::KeyNumpad6 },
        { VK_NUMPAD7, eKey::KeyNumpad7 },
        { VK_NUMPAD8, eKey::KeyNumpad8 },
        { VK_NUMPAD9, eKey::KeyNumpad9 },
        { VK_ADD, eKey::KeyNumpadAdd },
        { VK_DIVIDE, eKey::KeyNumpadDiv },
        { VK_DECIMAL, eKey::KeyNumpadDot },
        { VK_SUBTRACT, eKey::KeyNumpadMinus },
        { VK_MULTIPLY, eKey::KeyNumpadMul },

        { VK_MENU, eKey::KeyAlt },
        { VK_OEM_7, eKey::KeyApostrophe },
        { VK_OEM_5, eKey::KeyBackslash },
        { VK_BACK, eKey::KeyBackspace },
        { VK_CAPITAL, eKey::KeyCapsLock },
        { VK_OEM_COMMA, eKey::KeyComma },
        { VK_CONTROL, eKey::KeyCtrl },
        { VK_DELETE, eKey::KeyDel },
        { VK_END, eKey::KeyEnd },
        { VK_RETURN, eKey::KeyEnter },
        { VK_ESCAPE, eKey::KeyEsc },
        { VK_HOME, eKey::KeyHome },
        { VK_INSERT, eKey::KeyIns },
        { VK_APPS, eKey::KeyMenu },
        { VK_OEM_MINUS, eKey::KeyMinus },
        { VK_PAUSE, eKey::KeyPause },
        { VK_OEM_PERIOD, eKey::KeyPeriod },
        { VK_NEXT, eKey::KeyPgDown },
        { VK_PRIOR, eKey::KeyPgUp },
        { VK_OEM_PLUS, eKey::KeyPlus },
        { VK_OEM_1, eKey::KeySemicolon },
        { VK_SHIFT, eKey::KeyShift },
        { VK_OEM_2, eKey::KeySlash },
        { VK_SPACE, eKey::KeySpace },
        { VK_TAB, eKey::KeyTab },
        { VK_OEM_3, eKey::KeyTilde }
    };
}

bool MainWindow::MakeWindow ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Making OS window" )

    _messageQueue = &messageQueue;
    HMODULE const module = GetModuleHandleA ( nullptr );
    constexpr int exeIconResourceID = 1;

    CreateCursors ();

    WNDCLASSA const wndClass
    {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
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

void MainWindow::OnKeyboardKey ( WPARAM wParam, eMessageType messageType ) noexcept
{
    auto const key = _keyboardKeyMapper.find ( wParam );

    if ( key == _keyboardKeyMapper.cend () ) [[unlikely]]
    {
        android_vulkan::LogDebug ( ">>> 0x%04X", wParam );
        return;
    }

    KeyboardKeyEvent event ( key->second );

    constexpr uint16_t pressedMask = 0b1000'0000'0000'0000U;
    KeyModifier &m = event._modifier;

    m._leftAlt = pressedMask & static_cast<uint16_t> ( GetKeyState ( VK_LMENU ) );
    m._rightAlt = pressedMask & static_cast<uint16_t> ( GetKeyState ( VK_RMENU ) );

    m._leftCtrl = pressedMask & static_cast<uint16_t> ( GetKeyState ( VK_LCONTROL ) );
    m._rightCtrl = pressedMask & static_cast<uint16_t> ( GetKeyState ( VK_RCONTROL ) );

    m._leftShift = pressedMask & static_cast<uint16_t> ( GetKeyState ( VK_LSHIFT ) );
    m._rightShift = pressedMask & static_cast<uint16_t> ( GetKeyState ( VK_RSHIFT ) );

    _messageQueue->EnqueueBack ( event.Create ( messageType ) );
}

void MainWindow::OnMouseKey ( LPARAM lParam, eKey key, eMessageType messageType ) noexcept
{
    _messageQueue->EnqueueBack (
        {
            ._type = messageType,

            ._params = new MouseKeyEvent
            {
                ._x = static_cast<int32_t> ( GET_X_LPARAM ( lParam ) ),
                ._y = static_cast<int32_t> ( GET_Y_LPARAM ( lParam ) ),
                ._key = key
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
        std::make_pair ( eCursor::IBeam,
            static_cast<HCURSOR> ( LoadImageW ( nullptr, IDC_IBEAM, IMAGE_CURSOR, 0, 0, flags ) )
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

        case WM_KEYDOWN:
            [[fallthrough]];
        case WM_SYSKEYDOWN:
            mainWindow.OnKeyboardKey ( wParam, eMessageType::KeyboardKeyDown );
        return 0;

        case WM_KEYUP:
            [[fallthrough]];
        case WM_SYSKEYUP:
            mainWindow.OnKeyboardKey ( wParam, eMessageType::KeyboardKeyUp );
        return 0;

        case WM_LBUTTONDOWN:
            mainWindow.OnMouseKey ( lParam, eKey::LeftMouseButton, eMessageType::MouseButtonDown );
        return 0;

        case WM_LBUTTONUP:
            // [2025/03/30] Win11 24H2 26100.3624 will send WM_MOUSEMOVE event right after WM_LBUTTONUP automatically.
            mainWindow.OnMouseKey ( lParam, eKey::LeftMouseButton, eMessageType::MouseButtonUp );
        return 0;

        case WM_MBUTTONDOWN:
            mainWindow.OnMouseKey ( lParam, eKey::MiddleMouseButton, eMessageType::MouseButtonDown );
        return 0;

        case WM_MBUTTONUP:
            mainWindow.OnMouseKey ( lParam, eKey::MiddleMouseButton, eMessageType::MouseButtonUp );
        return 0;

        case WM_MOUSEMOVE:
            mainWindow.OnMouseMove ( lParam );
        break;

        case WM_RBUTTONDOWN:
            mainWindow.OnMouseKey ( lParam, eKey::RightMouseButton, eMessageType::MouseButtonDown );
        return 0;

        case WM_RBUTTONUP:
            mainWindow.OnMouseKey ( lParam, eKey::RightMouseButton, eMessageType::MouseButtonUp );
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
