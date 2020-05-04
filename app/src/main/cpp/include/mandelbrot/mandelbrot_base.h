#ifndef MANDELBROT_BASE_H
#define MANDELBROT_BASE_H


#include <game.h>


namespace mandelbrot {

class MandelbrotBase : public android_vulkan::Game
{
    protected:
        // Note VkImage is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkCommandPool                   _commandPool;

        // Note VkPipeline is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkPipeline                      _pipeline;

        // Note VkPipelineLayout is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkPipelineLayout                _pipelineLayout;

        // Note VkRenderPass is a VK_DEFINE_NON_DISPATCHABLE_HANDLE.
        VkRenderPass                    _renderPass;

    private:
        // Note VkFence is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkFence                         _presentationFence;

        // Note VkSemaphore is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkSemaphore                     _presentationSemaphore;

        // Note VkShaderModule is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkShaderModule                  _vertexShader;

        VkShaderModule                  _fragmentShader;
        const char*                     _fragmentShaderSpirV;

    protected:
        std::vector<VkCommandBuffer>    _commandBuffer;

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
        bool BeginFrame ( uint32_t &presentationImageIndex, android_vulkan::Renderer &renderer );
        bool EndFrame ( uint32_t presentationImageIndex, android_vulkan::Renderer &renderer );

        bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        bool DestroyCommandPool ( android_vulkan::Renderer &renderer );

        bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer );
        void DestroyPresentationSyncPrimitive ( android_vulkan::Renderer &renderer );

        bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( android_vulkan::Renderer &renderer );

        bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( android_vulkan::Renderer &renderer );
};

} // namespace mandelbrot


#endif // MANDELBROT_BASE_H
