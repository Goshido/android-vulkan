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
        const char*                     _fragmentShaderSpirV;

    public:
        MandelbrotBase ( const MandelbrotBase &other ) = delete;
        MandelbrotBase& operator = ( const MandelbrotBase &other ) = delete;

        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

    protected:
        MandelbrotBase ( const char* fragmentShaderFile );
        ~MandelbrotBase () override = default;

        virtual bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) = 0;
        virtual void DestroyPipelineLayout ( android_vulkan::Renderer &renderer ) = 0;

    private:
        bool BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationImageIndex );
        bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex );

        bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        bool DestroyCommandPool ( android_vulkan::Renderer &renderer );

        bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        bool DestroyFramebuffers ( android_vulkan::Renderer &renderer );

        bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer );
        void DestroyPresentationSyncPrimitive ( android_vulkan::Renderer &renderer );

        bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( android_vulkan::Renderer &renderer );

        bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( android_vulkan::Renderer &renderer );
};

} // namespace mandelbrot


#endif // MANDELBROT_BASE_H
