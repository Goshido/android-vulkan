#ifndef ANDROID_VULKAN_TRACE_HPP
#define ANDROID_VULKAN_TRACE_HPP


#ifndef ANDROID_ENABLE_TRACE

#define AV_TRACE(name)

#else

#define AV_TRACE(name) android_vulkan::Trace const trace ( name );

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

} // namespace android_vulkan

#endif // ANDROID_ENABLE_TRACE


#endif // ANDROID_VULKAN_TRACE_HPP
