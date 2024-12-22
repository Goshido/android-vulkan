#include <precompiled_headers.hpp>

#ifdef AV_ENABLE_TRACE

#include <trace.hpp>
#include <GXCommon/GXWarning.hpp>


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


#endif // AV_ENABLE_TRACE
