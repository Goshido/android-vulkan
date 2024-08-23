#ifdef ANDROID_ENABLE_TRACE

#include <trace.hpp>
#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

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

} // namespace android_vulkan

#endif // ANDROID_ENABLE_TRACE
