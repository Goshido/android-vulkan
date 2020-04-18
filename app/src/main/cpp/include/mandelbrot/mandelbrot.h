#ifndef MANDELBROT_H
#define MANDELBROT_H


#include <game.h>


namespace mandelbrot {

class Mandelbrot final : public android_vulkan::Game
{
    private:
        // Note VkImage is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkCommandPool                   _commandPool;

        // Note VkPipeline is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkPipeline                      _pipeline;

        // Note VkPipelineLayout is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkPipelineLayout                _pipelineLayout;

        // Note VkFence is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkFence                         _presentationFence;

        // Note VkSemaphore is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkSemaphore                     _presentationSemaphore;

        // Note VkRenderPass is a VK_DEFINE_NON_DISPATCHABLE_HANDLE.
        VkRenderPass                    _renderPass;

        // Note VkShaderModule is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkShaderModule                  _vertexShader;
        VkShaderModule                  _fragmentShader;

        std::vector<VkCommandBuffer>    _commandBuffer;

    public:
        Mandelbrot ();
        ~Mandelbrot () override = default;

        Mandelbrot ( const Mandelbrot &other ) = delete;
        Mandelbrot& operator = ( const Mandelbrot &other ) = delete;

        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

    private:
        bool BeginFrame ( uint32_t &presentationImageIndex, android_vulkan::Renderer &renderer );
        bool EndFrame ( uint32_t presentationImageIndex, android_vulkan::Renderer &renderer );

        bool CreateCommandBuffer ( android_vulkan::Renderer &renderer );
        void DestroyCommandBuffer ( android_vulkan::Renderer &renderer );

        bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer );
        void DestroyPresentationSyncPrimitive ( android_vulkan::Renderer &renderer );

        bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( android_vulkan::Renderer &renderer );

        bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( android_vulkan::Renderer &renderer );

        bool CreateShader ( VkShaderModule &shader, const char* shaderFile, android_vulkan::Renderer &renderer ) const;
};

} // namespace mandelbrot


#endif // MANDELBROT_H
