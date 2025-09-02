// FUCK - remove namespace
#ifndef PBR_WINDOWS_PRESENT_PASS_HPP
#define PBR_WINDOWS_PRESENT_PASS_HPP


#include <renderer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


// FUCK - remove namespace
namespace pbr::windows {

class PresentPass final
{
    private:
        constexpr static VkPipelineStageFlags WAIT_STAGE = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    private:
        uint32_t                        _swapchainImageIndex = std::numeric_limits<uint32_t>::max ();
        std::vector<VkSemaphore>        _renderEnd {};

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

        void OnDestroyDevice ( VkDevice device ) noexcept;
        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept;

        void Begin ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] std::optional<VkResult> End ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkSemaphore acquire,
            VkFence fence,
            std::mutex* submitMutex
        ) noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_PRESENT_PASS_HPP
