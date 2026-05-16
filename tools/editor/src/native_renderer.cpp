#include <precompiled_headers.hpp>
#include <native_renderer.hpp>


namespace editor {

NativeRenderer* NativeRenderer::_instance = nullptr;

NativeRenderer::NativeRenderer () noexcept
{
    _instance = this;
}

android_vulkan::Renderer &NativeRenderer::Instance () noexcept
{
    return _instance->_renderer;
}

} // namespace editor
