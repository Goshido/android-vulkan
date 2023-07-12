#ifndef MANDELBROT_BASE_HPP
#define MANDELBROT_BASE_HPP


#include <game.hpp>


namespace mandelbrot {

class MandelbrotBase : public android_vulkan::Game
{
    protected:
        std::vector<VkCommandBuffer>    _commandBuffer {};
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        std::vector<VkFramebuffer>      _framebuffers {};
        VkPipeline                      _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout                _pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass                    _renderPass = VK_NULL_HANDLE;

    private:
        std::vector<VkFence>            _fences {};

        VkSemaphore                     _renderPassEndedSemaphore = VK_NULL_HANDLE;
        VkSemaphore                     _renderTargetAcquiredSemaphore = VK_NULL_HANDLE;

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

        [[nodiscard]] virtual bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept = 0;
        virtual void DestroyPipelineLayout ( VkDevice device ) noexcept = 0;

    private:
        [[nodiscard]] bool IsReady () override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationImageIndex ) noexcept;
        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex ) noexcept;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFences ( VkDevice device ) noexcept;
        void DestroyFences ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyFramebuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyPresentationSyncPrimitive ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreatePipeline ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyPipeline ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyRenderPass ( VkDevice device ) noexcept;

        void DestroyShaderModules ( VkDevice device ) noexcept;
};

} // namespace mandelbrot


#endif // MANDELBROT_BASE_HPP
