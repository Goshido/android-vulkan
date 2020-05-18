#ifndef RAINBOW_H
#define RAINBOW_H


#include <game.h>


namespace rainbow {

class Rainbow final : public android_vulkan::Game
{
    private:
        using CommandContext = std::pair<VkCommandBuffer, VkFence>;

        std::vector<CommandContext>     _commandBuffers;
        VkCommandPool                   _commandPool;
        std::vector<VkFramebuffer>      _framebuffers;
        VkRenderPass                    _renderPass;
        VkSemaphore                     _renderPassEndedSemaphore;
        VkSemaphore                     _renderTargetAcquiredSemaphore;

    public:
        Rainbow ();
        ~Rainbow () override = default;

        Rainbow ( const Rainbow &other ) = delete;
        Rainbow& operator = ( const Rainbow &other ) = delete;

        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

    private:
        bool BeginFrame ( uint32_t &presentationFramebufferIndex, android_vulkan::Renderer &renderer );
        bool EndFrame ( uint32_t presentationFramebufferIndex, android_vulkan::Renderer &renderer );

        bool CreateCommandBuffer ( android_vulkan::Renderer &renderer );
        void DestroyCommandBuffer ( android_vulkan::Renderer &renderer );

        bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( android_vulkan::Renderer &renderer );

        bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer );
        void DestroyPresentationSyncPrimitive ( android_vulkan::Renderer &renderer );

        bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( android_vulkan::Renderer &renderer );
};

} // namespace rainbow


#endif // RAINBOW_H
