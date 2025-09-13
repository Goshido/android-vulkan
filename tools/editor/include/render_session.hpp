#ifndef EDITOR_RENDER_SESSION_HPP
#define EDITOR_RENDER_SESSION_HPP


#include "hello_triangle_program.hpp"
#include "message_queue.hpp"
#include <pbr/default_texture_manager.hpp>

// FUCK - remove namespace
#include <platform/android/pbr/exposure_pass.hpp>
#include <platform/android/pbr/present_pass.hpp>
#include <platform/android/pbr/tone_mapper_pass.hpp>

#include <platform/windows/mesh_geometry.hpp>
#include <platform/windows/pbr/exposure_pass.hpp>
#include <platform/windows/pbr/present_pass.hpp>
#include <platform/windows/pbr/resource_heap.hpp>
#include <platform/windows/pbr/tone_mapper_pass.hpp>
#include <platform/windows/pbr/ui_pass.hpp>
#include "ui_manager.hpp"


namespace editor {

class RenderSession final
{
    private:
        struct CommandInfo final
        {
            VkSemaphore                                             _acquire = VK_NULL_HANDLE;
            VkCommandBuffer                                         _buffer = VK_NULL_HANDLE;
            VkFence                                                 _fence = VK_NULL_HANDLE;
            bool                                                    _inUse = false;
            VkCommandPool                                           _pool = VK_NULL_HANDLE;
        };

        using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

    private:
        bool                                                        _broken = false;

        CommandInfo                                                 _commandInfo[ pbr::FIF_COUNT ];
        size_t                                                      _writingCommandInfo = 0U;

        pbr::DefaultTextureManager                                  _defaultTextureManager {};

        // FUCK - remove namespace
        pbr::android::ExposurePass                                  _exposurePass {};
        pbr::windows::ExposurePass                                  _exposurePassEXT {};

        std::unique_ptr<HelloTriangleProgram>                       _helloTriangleProgram {};

        // FUCK
        std::unique_ptr<android_vulkan::android::MeshGeometry>      _fuckHelloTriangleGeometry {};
        std::unique_ptr<android_vulkan::windows::MeshGeometry>      _helloTriangleGeometry {};

        MessageQueue                                                &_messageQueue;
        android_vulkan::Renderer                                    &_renderer;

        // FUCK - remove namespace
        pbr::android::PresentPass                                   _presentRenderPass {};
        pbr::windows::PresentPass                                   _presentRenderPassEXT {};

        // FUCK - refactor
        VkRenderPassBeginInfo                                       _renderPassInfo {};

        android_vulkan::Texture2D                                   _renderTarget {};
        uint32_t                                                    _renderTargetIdx = 0U;

        // FUCK - call update GPU
        pbr::windows::ResourceHeap                                  _resourceHeap {};

        // FUCK - remove
        pbr::SamplerManager                                         _samplerManager {};

        std::mutex                                                  _submitMutex {};
        std::thread                                                 _thread {};
        Timestamp                                                   _timestamp {};

        // FUCK - remove namespace
        pbr::android::ToneMapperPass                                _toneMapper {};
        pbr::windows::ToneMapperPass                                _toneMapperEXT {};

        size_t                                                      _uiElements = 0U;

        // FUCK - remove namespace
        pbr::android::UIPass                                        _uiPass {};
        pbr::windows::UIPass                                        _uiPassEXT { _resourceHeap };

        UIManager                                                   &_uiManager;
        VkViewport                                                  _viewport {};

    public:
        RenderSession () = delete;

        RenderSession ( RenderSession const & ) = delete;
        RenderSession &operator = ( RenderSession const & ) = delete;

        RenderSession ( RenderSession && ) = delete;
        RenderSession &operator = ( RenderSession && ) = delete;

        explicit RenderSession ( MessageQueue &messageQueue,
            android_vulkan::Renderer &renderer,
            UIManager &uiManager
        ) noexcept;

        ~RenderSession () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        // FUCK - remove namespace
        [[nodiscard]] pbr::android::FontStorage &GetFontStorage () noexcept;

    private:
        [[nodiscard]] bool AllocateCommandBuffers ( VkDevice device ) noexcept;
        void FreeCommandBuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFramebuffer ( VkDevice device, VkExtent2D const &resolution ) noexcept;
        [[nodiscard]] bool CreateRenderPass ( VkDevice device ) noexcept;
        [[nodiscard]] bool CreateRenderTarget () noexcept;
        [[nodiscard]] bool CreateRenderTargetImage ( VkExtent2D const &resolution ) noexcept;
        void EventLoop () noexcept;
        [[nodiscard]] bool InitModules () noexcept;

        void OnHelloTriangleReady ( void* params ) noexcept;
        void OnRenderFrame () noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
        void OnSwapchainCreated () noexcept;
        void OnUIAppendChildElement ( Message &&message ) noexcept;
        void OnUIDeleteElement ( Message &&message ) noexcept;
        void OnUIElementCreated () noexcept;
        void OnUIHideElement ( Message &&message ) noexcept;
        void OnUIShowElement ( Message &&message ) noexcept;
        void OnUIPrependChildElement ( Message &&message ) noexcept;
        void OnUISetText ( Message &&message ) noexcept;
        void OnUIUpdateElement ( Message &&message ) noexcept;

        void NotifyRecreateSwapchain () const noexcept;

        [[nodiscard]] static bool PrepareCommandBuffer ( VkDevice device, CommandInfo &info ) noexcept;
};

} // namespace editor


#endif // EDITOR_RENDER_SESSION_HPP
