#ifndef ANDROID_VULKAN_TRACE_HPP
#define ANDROID_VULKAN_TRACE_HPP


#ifndef AV_ENABLE_TRACE

#define AV_TRACE(...)
#define AV_THREAD_NAME(...)

#else

namespace android_vulkan {

class Trace final
{
    public:
        Trace () = delete;

        Trace ( Trace const & ) = delete;
        Trace &operator = ( Trace const & ) = delete;

        Trace ( Trace && ) = delete;
        Trace &operator = ( Trace && ) = delete;

        explicit Trace ( char const* name ) noexcept;

        ~Trace () noexcept;
};

void SetThreadName ( char const* name ) noexcept;

} // namespace android_vulkan

#define AV_TRACE_GEN(x, y) x##y
#define AV_TRACE_GEN2(x, y) AV_TRACE_GEN ( x, y )

#define AV_TRACE_IMPL(trace, buf, ...)                                                                                 \
    char buf[ 256U ];                                                                                                  \
    std::snprintf ( buf, std::size ( buf ), __VA_ARGS__ );                                                             \
    android_vulkan::Trace const trace ( buf );

#define AV_TRACE(...) AV_TRACE_IMPL ( AV_TRACE_GEN2 ( trace, __LINE__ ), AV_TRACE_GEN2 ( buf, __LINE__ ), __VA_ARGS__ )

#define AV_THREAD_NAME_IMPL(buf, ...)                                                                                  \
    char buf[ 256U ];                                                                                                  \
    std::snprintf ( buf, std::size ( buf ), __VA_ARGS__ );                                                             \
    android_vulkan::SetThreadName ( buf );

#define AV_THREAD_NAME(...) AV_THREAD_NAME_IMPL ( AV_TRACE_GEN2 ( buf, __LINE__ ), __VA_ARGS__ )

#endif // AV_ENABLE_TRACE


#endif // ANDROID_VULKAN_TRACE_HPP
