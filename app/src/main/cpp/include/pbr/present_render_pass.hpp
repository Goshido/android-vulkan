#ifndef PBR_PRESENT_RENDER_PASS_HPP
#define PBR_PRESENT_RENDER_PASS_HPP


#include "sampler_manager.hpp"


namespace pbr {

class PresentRenderPass final
{
    private:
        uint32_t                        _framebufferIndex = std::numeric_limits<uint32_t>::max ();
        std::vector<VkFramebuffer>      _framebuffers {};
        VkPresentInfoKHR                _presentInfo {};
        VkSemaphore                     _renderEndSemaphore = VK_NULL_HANDLE;
        VkRenderPassBeginInfo           _renderInfo {};
        VkSubmitInfo                    _submitInfo {};
        VkSemaphore                     _targetAcquiredSemaphore = VK_NULL_HANDLE;

    public:
        PresentRenderPass () = default;

        PresentRenderPass ( PresentRenderPass const & ) = delete;
        PresentRenderPass &operator = ( PresentRenderPass const & ) = delete;

        PresentRenderPass ( PresentRenderPass && ) = delete;
        PresentRenderPass &operator = ( PresentRenderPass && ) = delete;

        ~PresentRenderPass () = default;

        [[nodiscard]] bool AcquirePresentTarget ( android_vulkan::Renderer &renderer,
            size_t &swapchainImageIndex
        ) noexcept;

        [[nodiscard]] VkRenderPass GetRenderPass () const noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        void OnDestroyDevice ( VkDevice device ) noexcept;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept;

        void Begin ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool End ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] constexpr static uint32_t GetSubpass () noexcept
        {
            return 0U;
        }

    private:
        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer,
            VkDevice device,
            VkExtent2D const &resolution
        ) noexcept;

        void DestroyFramebuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer,
            VkDevice device,
            VkExtent2D const &resolution
        ) noexcept;

        void InitCommonStructures () noexcept;
};

} // namespace pbr


#endif // PBR_PRESENT_RENDER_PASS_HPP
