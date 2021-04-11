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
        Rainbow () noexcept;

        Rainbow ( Rainbow const & ) = delete;
        Rainbow& operator = ( Rainbow const & ) = delete;

        Rainbow ( Rainbow && ) = delete;
        Rainbow& operator = ( Rainbow && ) = delete;

        ~Rainbow () override = default;

    private:
        [[nodiscard]] bool IsReady () override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) override;
        void OnDestroyDevice ( VkDevice device ) override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) override;
        void OnSwapchainDestroyed ( VkDevice device ) override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationFramebufferIndex );
        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationFramebufferIndex );

        [[nodiscard]] bool CreateCommandBuffer ( android_vulkan::Renderer &renderer );
        void DestroyCommandBuffer ( VkDevice device );

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( VkDevice device );

        [[nodiscard]] bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer );
        void DestroyPresentationSyncPrimitive ( VkDevice device );

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( VkDevice device );
};

} // namespace rainbow


#endif // RAINBOW_H
