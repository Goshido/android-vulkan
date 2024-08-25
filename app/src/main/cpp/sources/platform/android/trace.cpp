#ifdef ANDROID_ENABLE_TRACE


#include <trace.hpp>
#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <android/trace.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

Trace::Trace ( char const* name ) noexcept
{
    ATrace_beginSection ( name );
}

Trace::~Trace () noexcept
{
    ATrace_endSection ();
}

//----------------------------------------------------------------------------------------------------------------------

void SetThreadName ( char const* /*name*/ ) noexcept
{
    // TODO
}

} // namespace android_vulkan


#endif // ANDROID_ENABLE_TRACE
