#ifndef ANDROID_VULKAN_TRACE_HPP
#define ANDROID_VULKAN_TRACE_HPP


#ifndef AV_ENABLE_TRACE

#define AV_TRACE(name)
#define AV_THREAD_NAME(name)

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

#define AV_TRACE(name) android_vulkan::Trace const _FuCk_TrAcE_ ( name );
#define AV_THREAD_NAME(name) android_vulkan::SetThreadName ( name );

#endif // AV_ENABLE_TRACE


#endif // ANDROID_VULKAN_TRACE_HPP
