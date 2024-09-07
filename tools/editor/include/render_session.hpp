#ifndef EDITOR_RENDER_SESSION_HPP
#define EDITOR_RENDER_SESSION_HPP


#include "blit_program.hpp"
#include <main_window.hpp>
#include "message_queue.hpp"
#include <pbr/command_buffer_count.hpp>
#include <pbr/present_render_pass.hpp>
#include <pbr/sampler.hpp>
#include <renderer.hpp>
#include <texture2D.hpp>


namespace editor {

class RenderSession final
{
    private:
        struct CommandInfo final
        {
            VkCommandBuffer             _buffer = VK_NULL_HANDLE;
            VkFence                     _fence = VK_NULL_HANDLE;
            VkSemaphore                 _acquire = VK_NULL_HANDLE;
            VkCommandPool               _pool = VK_NULL_HANDLE;
        };

    private:
        VkDescriptorPool                _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                 _descriptorSet = VK_NULL_HANDLE;

        BlitDescriptorSetLayout         _blitLayout {};
        BlitProgram                     _blitProgram {};
        bool                            _broken = false;

        CommandInfo                     _commandInfo[ pbr::DUAL_COMMAND_BUFFER ];
        size_t                          _writingCommandInfo = 0U;

        MessageQueue*                   _messageQueue = nullptr;
        android_vulkan::Renderer*       _renderer = nullptr;
        pbr::Sampler                    _sampler {};
        std::thread                     _thread {};
        pbr::PresentRenderPass          _presentRenderPass {};
        VkRenderPassBeginInfo           _renderPassInfo {};
        android_vulkan::Texture2D       _renderTarget {};
        VkViewport                      _viewport {};

    public:
        explicit RenderSession () = default;

        RenderSession ( RenderSession const & ) = delete;
        RenderSession &operator = ( RenderSession const & ) = delete;

        RenderSession ( RenderSession && ) = delete;
        RenderSession &operator = ( RenderSession && ) = delete;

        ~RenderSession () = default;

        [[nodiscard]] bool Init ( MessageQueue &messageQueue, android_vulkan::Renderer &renderer ) noexcept;
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

        void OnRenderFrame () noexcept;
        void OnShutdown () noexcept;
        void OnSwapchainCreated () noexcept;

        void NotifyRecreateSwapchain () const noexcept;
};

} // namespace editor


#endif // EDITOR_RENDER_SESSION_HPP
