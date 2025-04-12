#ifndef EDITOR_RENDER_SESSION_HPP
#define EDITOR_RENDER_SESSION_HPP


#include "hello_triangle_program.hpp"
#include "message_queue.hpp"
#include <pbr/default_texture_manager.hpp>
#include <pbr/exposure_pass.hpp>
#include <pbr/present_render_pass.hpp>
#include <pbr/tone_mapper_pass.hpp>
#include <pbr/ui_pass.hpp>
#include "ui_manager.hpp"


namespace editor {

class RenderSession final
{
    private:
        struct CommandInfo final
        {
            VkSemaphore                                     _acquire = VK_NULL_HANDLE;
            VkCommandBuffer                                 _buffer = VK_NULL_HANDLE;
            VkFence                                         _fence = VK_NULL_HANDLE;
            bool                                            _inUse = false;
            VkCommandPool                                   _pool = VK_NULL_HANDLE;
        };

        using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

    private:
        bool                                                _broken = false;

        CommandInfo                                         _commandInfo[ pbr::DUAL_COMMAND_BUFFER ];
        size_t                                              _writingCommandInfo = 0U;

        pbr::DefaultTextureManager                          _defaultTextureManager {};
        pbr::ExposurePass                                   _exposurePass {};

        std::unique_ptr<HelloTriangleProgram>               _helloTriangleProgram {};
        std::unique_ptr<android_vulkan::MeshGeometry>       _helloTriangleGeometry {};

        MessageQueue                                        &_messageQueue;
        android_vulkan::Renderer                            &_renderer;
        pbr::PresentRenderPass                              _presentRenderPass {};
        VkRenderPassBeginInfo                               _renderPassInfo {};
        android_vulkan::Texture2D                           _renderTarget {};
        pbr::SamplerManager                                 _samplerManager {};
        std::mutex                                          _submitMutex {};
        std::thread                                         _thread {};
        Timestamp                                           _timestamp {};
        pbr::ToneMapperPass                                 _toneMapper {};
        size_t                                              _uiElements = 0U;
        pbr::UIPass                                         _uiPass {};
        UIManager                                           &_uiManager;
        VkViewport                                          _viewport {};

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

    private:
        [[nodiscard]] bool AllocateCommandBuffers ( VkDevice device ) noexcept;
        void FreeCommandBuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFramebuffer ( VkDevice device, VkExtent2D const &resolution ) noexcept;
        [[nodiscard]] bool CreateRenderPass ( VkDevice device ) noexcept;
        [[nodiscard]] bool CreateRenderTarget () noexcept;
        [[nodiscard]] bool CreateRenderTargetImage ( VkExtent2D const &resolution ) noexcept;
        void EventLoop () noexcept;
        [[nodiscard]] bool InitiModules () noexcept;

        void OnHelloTriangleReady ( void* params ) noexcept;
        void OnRenderFrame () noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
        void OnSwapchainCreated () noexcept;
        void OnUIAppendChildElement ( Message &&message ) noexcept;
        void OnUIDeleteElement ( Message &&message ) noexcept;
        void OnUIElementCreated () noexcept;
        void OnUIPrependChildElement ( Message &&message ) noexcept;
        void OnUISetText ( Message &&message ) noexcept;

        void NotifyRecreateSwapchain () const noexcept;

        [[nodiscard]] static bool PrepareCommandBuffer ( VkDevice device, CommandInfo &info ) noexcept;
};

} // namespace editor


#endif // EDITOR_RENDER_SESSION_HPP
