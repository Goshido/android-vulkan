#ifndef PBR_PRESENT_PASS_HPP
#define PBR_PRESENT_PASS_HPP


#include <renderer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr {

class PresentPass final
{
    private:
        constexpr static VkPipelineStageFlags WAIT_STAGE = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        struct FramebufferInfo final
        {
            VkFramebuffer               _framebuffer = VK_NULL_HANDLE;
            VkSemaphore                 _renderEnd = VK_NULL_HANDLE;
        };

        uint32_t                        _framebufferIndex = std::numeric_limits<uint32_t>::max ();
        std::vector<FramebufferInfo>    _framebufferInfo {};

        VkRenderPassBeginInfo           _renderInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = VK_NULL_HANDLE,
            .framebuffer = VK_NULL_HANDLE,

            .renderArea
            {
                .offset
                {
                    .x = 0,
                    .y = 0
                },

                .extent
                {
                    .width = 0U,
                    .height = 0U
                }
            },

            .clearValueCount = 0U,
            .pClearValues = nullptr
        };

        VkPresentInfoKHR                _presentInfo
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1U,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1U,
            .pSwapchains = nullptr,
            .pImageIndices = nullptr,
            .pResults = nullptr
        };

        VkSubmitInfo                    _submitInfo
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1U,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = &WAIT_STAGE,
            .commandBufferCount = 1U,
            .pCommandBuffers = nullptr,
            .signalSemaphoreCount = 1U,
            .pSignalSemaphores = nullptr
        };

    public:
        explicit PresentPass () = default;

        PresentPass ( PresentPass const & ) = delete;
        PresentPass &operator = ( PresentPass const & ) = delete;

        PresentPass ( PresentPass && ) = delete;
        PresentPass &operator = ( PresentPass && ) = delete;

        ~PresentPass () = default;

        [[nodiscard]] VkResult AcquirePresentTarget ( android_vulkan::Renderer &renderer,
            VkSemaphore acquire
        ) noexcept;

        [[nodiscard]] VkRenderPass GetRenderPass () const noexcept;

        void OnDestroyDevice ( VkDevice device ) noexcept;
        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept;

        void Begin ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] std::optional<VkResult> End ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkSemaphore acquire,
            VkFence fence,
            std::mutex* submitMutex
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
};

} // namespace pbr


#endif // PBR_PRESENT_PASS_HPP
