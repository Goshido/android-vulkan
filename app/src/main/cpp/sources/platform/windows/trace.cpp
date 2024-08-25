#ifdef ANDROID_ENABLE_TRACE

#include <trace.hpp>
#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <Windows.h>
#include <WinPixEventRuntime/pix3.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

Trace::Trace ( char const* name ) noexcept
{
    PIXBeginEvent ( PIX_COLOR_DEFAULT, "%s", name );
}

Trace::~Trace () noexcept
{
    PIXEndEvent ();
}

//----------------------------------------------------------------------------------------------------------------------

void SetThreadName ( char const* name ) noexcept
{
    wchar_t ssoBuffer[ 256U ];
    constexpr auto ssoLen = static_cast<int> ( std::size ( ssoBuffer ) );
    int const needed = MultiByteToWideChar ( 0U, 0U, name, -1, ssoBuffer, ssoLen );

    if ( needed <= ssoLen ) [[likely]]
    {
        SetThreadDescription ( GetCurrentThread (), ssoBuffer );
        return;
    }

    std::wstring wName{};
    wName.resize ( static_cast<size_t> ( needed ) );
    wchar_t* dst = wName.data ();
    MultiByteToWideChar ( 0U, 0U, name, -1, dst, needed );

    SetThreadDescription ( GetCurrentThread (), dst );
}

} // namespace android_vulkan

#endif // ANDROID_ENABLE_TRACE
