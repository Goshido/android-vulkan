#ifndef EDITOR_NATIVE_RENDERER_HPP
#define EDITOR_NATIVE_RENDERER_HPP


#include <renderer.hpp>


namespace editor {

class NativeRenderer final
{
    private:
        android_vulkan::Renderer    _renderer {};

        static NativeRenderer*      _instance;

    public:
        explicit NativeRenderer () noexcept;

        NativeRenderer ( NativeRenderer const & ) = delete;
        NativeRenderer &operator = ( NativeRenderer const & ) = delete;

        NativeRenderer ( NativeRenderer && ) = delete;
        NativeRenderer &operator = ( NativeRenderer && ) = delete;

        ~NativeRenderer () = default;

        [[nodiscard]] static android_vulkan::Renderer &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_NATIVE_RENDERER_HPP
