#ifndef MANDELBROT_BASE_H
#define MANDELBROT_BASE_H


#include <game.h>


namespace mandelbrot {

class MandelbrotBase : public android_vulkan::Game
{
    protected:
        std::vector<VkCommandBuffer>    _commandBuffer;
        VkCommandPool                   _commandPool;
        std::vector<VkFramebuffer>      _framebuffers;
        VkPipeline                      _pipeline;
        VkPipelineLayout                _pipelineLayout;
        VkRenderPass                    _renderPass;

    private:
        VkSemaphore                     _renderPassEndedSemaphore;
        VkSemaphore                     _renderTargetAcquiredSemaphore;

        VkShaderModule                  _vertexShader;

        VkShaderModule                  _fragmentShader;
        char const*                     _fragmentShaderSpirV;

    public:
        MandelbrotBase ( MandelbrotBase const & ) = delete;
        MandelbrotBase& operator = ( MandelbrotBase const & ) = delete;

        MandelbrotBase ( MandelbrotBase && ) = delete;
        MandelbrotBase& operator = ( MandelbrotBase && ) = delete;

    protected:
        explicit MandelbrotBase ( const char* fragmentShaderFile ) noexcept;
        ~MandelbrotBase () override = default;

        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( VkDevice device ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept override;

        [[nodiscard]] virtual bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) = 0;
        virtual void DestroyPipelineLayout ( VkDevice device ) = 0;

    private:
        [[nodiscard]] bool IsReady () override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationImageIndex );
        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex );

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        void DestroyCommandPool ( VkDevice device );

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( VkDevice device );

        [[nodiscard]] bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer );
        void DestroyPresentationSyncPrimitive ( VkDevice device );

        [[nodiscard]] bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( VkDevice device );

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( VkDevice device );
};

} // namespace mandelbrot


#endif // MANDELBROT_BASE_H
