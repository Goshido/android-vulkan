#ifndef MANDELBROT_BASE_HPP
#define MANDELBROT_BASE_HPP


#include <game.hpp>


namespace mandelbrot {

class MandelbrotBase : public android_vulkan::Game
{
    protected:
        struct CommandInfo final
        {
            VkCommandPool               _pool = VK_NULL_HANDLE;
            VkCommandBuffer             _buffer = VK_NULL_HANDLE;
            VkFence                     _fence = VK_NULL_HANDLE;
            VkSemaphore                 _acquire = VK_NULL_HANDLE;
        };

        struct FramebufferInfo final
        {
            VkFramebuffer               _framebuffer = VK_NULL_HANDLE;
            VkSemaphore                 _renderEnd = VK_NULL_HANDLE;
        };

        constexpr static size_t         DUAL_COMMAND_BUFFER = 2U;

        std::vector<CommandInfo>        _commandInfo {};
        size_t                          _writeCommandInfoIndex = 0U;

        std::vector<FramebufferInfo>    _framebufferInfo{};

        VkPipeline                      _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout                _pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass                    _renderPass = VK_NULL_HANDLE;

    private:
        VkShaderModule                  _vertexShader = VK_NULL_HANDLE;

        VkShaderModule                  _fragmentShader = VK_NULL_HANDLE;
        char const*                     _fragmentShaderSpirV;

    public:
        MandelbrotBase () = delete;

        MandelbrotBase ( MandelbrotBase const & ) = delete;
        MandelbrotBase &operator = ( MandelbrotBase const & ) = delete;

        MandelbrotBase ( MandelbrotBase && ) = delete;
        MandelbrotBase &operator = ( MandelbrotBase && ) = delete;

    protected:
        explicit MandelbrotBase ( char const* fragmentShaderFile ) noexcept;
        ~MandelbrotBase () override = default;

        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] virtual bool RecordCommandBuffer ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFramebuffer framebuffer
        ) noexcept = 0;

        [[nodiscard]] virtual bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept = 0;
        virtual void DestroyPipelineLayout ( VkDevice device ) noexcept = 0;

    private:
        [[nodiscard]] bool IsReady () override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer,
            VkSemaphore acquire,
            uint32_t &presentationImageIndex
        ) noexcept;

        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex ) noexcept;

        [[nodiscard]] bool CreateCommandBuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandBuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyFramebuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreatePipeline ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyPipeline ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyRenderPass ( VkDevice device ) noexcept;

        void DestroyShaderModules ( VkDevice device ) noexcept;
};

} // namespace mandelbrot


#endif // MANDELBROT_BASE_HPP
