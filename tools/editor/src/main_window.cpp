#include <logger.hpp>
#include <main_window.hpp>
#include <trace.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <tchar.h>

GX_RESTORE_WARNING_STATE


namespace editor {

bool MainWindow::MakeWindow () noexcept
{
    AV_TRACE ( "Making OS window" )
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

    // Note: HWND handle will be assigned later in the WM_CREATE handler.
    HWND const handle = CreateWindowEx ( WS_EX_ACCEPTFILES | WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW,
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

    if ( handle ) [[likely]]
        return true;

    android_vulkan::LogError ( "MainWindow: Can't create native window." );
    return false;
}

bool MainWindow::MakeSwapchain () noexcept
{
    // FUCK
    return true;
}

bool MainWindow::Destroy () noexcept
{
    AV_TRACE ( "Destroying main window" )

    if ( _nativeWindow ) [[likely]]
    {
        BOOL const result = DestroyWindow ( _nativeWindow );

        if ( result == 0 ) [[unlikely]]
        {
            android_vulkan::LogError ( "MainWindow: Can't close native window." );
            return false;
        }

        Execute ();
        _nativeWindow = nullptr;
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

    while ( PeekMessageA ( &msg, _nativeWindow, 0U, 0U, PM_REMOVE ) )
    {
        DispatchMessageA ( &msg );
    }
}

void MainWindow::Connect ( HWND nativeWindow ) noexcept
{
    _nativeWindow = nativeWindow;

    // To destinguish error it's needed to preset 0 as last error. Trick is described on MSDN:
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptra#return-value
    SetLastError ( 0U );

    LONG_PTR const result = SetWindowLongPtrA ( nativeWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR> ( this ) );

    if ( ( result == 0 ) & ( GetLastError () == 0U ) ) [[likely]]
        return;

    android_vulkan::LogError ( "MainWindow: Can't connect MainWindow onject to HWND handle." );
    DestroyWindow ( nativeWindow );
}

void MainWindow::FUCKCheck () noexcept
{
    android_vulkan::LogDebug ( "Resize event %p (HWND %p)", this, _nativeWindow );
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
            reinterpret_cast<MainWindow*> ( createInfo.lpCreateParams )->Connect ( hwnd );
            return DefWindowProcA ( hwnd, msg, wParam, lParam );
        }

        case WM_SIZE:
            mainWindow.FUCKCheck ();
        return DefWindowProcA ( hwnd, msg, wParam, lParam );

        default:
        return DefWindowProcA ( hwnd, msg, wParam, lParam );
    }
}

} // namespace editor
