#ifndef RAINBOW_H
#define RAINBOW_H


#include <game.h>


namespace rainbow {

class Rainbow final : public android_vulkan::Game
{
    private:
        VkCommandBuffer                 _commandBuffer = VK_NULL_HANDLE;
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        VkFence                         _fence = VK_NULL_HANDLE;
        std::vector<VkFramebuffer>      _framebuffers {};
        VkRenderPass                    _renderPass = VK_NULL_HANDLE;
        VkSemaphore                     _renderPassEndedSemaphore = VK_NULL_HANDLE;
        VkSemaphore                     _renderTargetAcquiredSemaphore = VK_NULL_HANDLE;

    public:
        Rainbow () = default;

        Rainbow ( Rainbow const & ) = delete;
        Rainbow& operator = ( Rainbow const & ) = delete;

        Rainbow ( Rainbow && ) = delete;
        Rainbow& operator = ( Rainbow && ) = delete;

        ~Rainbow () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationFramebufferIndex ) noexcept;
        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationFramebufferIndex ) noexcept;

        [[nodiscard]] bool CreateCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandBuffer ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyFramebuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyPresentationSyncPrimitive ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyRenderPass ( VkDevice device ) noexcept;
};

} // namespace rainbow


#endif // RAINBOW_H
