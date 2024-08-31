#ifndef EDITOR_RENDER_SESSION_HPP
#define EDITOR_RENDER_SESSION_HPP


#include "message_queue.hpp"
#include <pbr/command_buffer_count.hpp>
#include <pbr/present_render_pass.hpp>
#include <renderer.hpp>


namespace editor {

class RenderSession final
{
    private:
        struct CommandInfo final
        {
            VkCommandBuffer             _buffer = VK_NULL_HANDLE;
            VkFence                     _fence = VK_NULL_HANDLE;
            VkCommandPool               _pool = VK_NULL_HANDLE;
            VkSemaphore                 _acquire = VK_NULL_HANDLE;
        };

    private:
        bool                            _broken = false;

        CommandInfo                     _commandInfo[ pbr::DUAL_COMMAND_BUFFER ];
        size_t                          _writingCommandInfo = 0U;

        MessageQueue*                   _messageQueue = nullptr;
        android_vulkan::Renderer*       _renderer = nullptr;
        std::thread                     _thread {};
        pbr::PresentRenderPass          _presentRenderPass {};

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
        [[nodiscard]] bool AllocateCommandBuffers () noexcept;
        void FreeCommandBuffers ( VkDevice device ) noexcept;

        void EventLoop () noexcept;
        [[nodiscard]] bool InitiModules () noexcept;
        void RenderFrame () noexcept;
        void Shutdown () noexcept;
};

} // namespace editor


#endif // EDITOR_RENDER_SESSION_HPP
