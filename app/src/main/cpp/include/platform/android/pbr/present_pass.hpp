// FUCK - remove namespace
#ifndef PBR_ANDROID_PRESENT_PASS_HPP
#define PBR_ANDROID_PRESENT_PASS_HPP


#include <renderer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


// FUCK - remove namespace
namespace pbr::android {

class PresentPass final
{
    private:
        struct FramebufferInfo final
        {
            VkFramebuffer               _framebuffer = VK_NULL_HANDLE;
            VkSemaphore                 _renderEnd = VK_NULL_HANDLE;
        };

        uint32_t                        _framebufferIndex = std::numeric_limits<uint32_t>::max ();
        std::vector<FramebufferInfo>    _framebufferInfo {};
        VkPresentInfoKHR                _presentInfo {};
        VkRenderPassBeginInfo           _renderInfo {};
        VkSubmitInfo                    _submitInfo {};

    public:
        PresentPass () = default;

        PresentPass ( PresentPass const & ) = delete;
        PresentPass &operator = ( PresentPass const & ) = delete;

        PresentPass ( PresentPass && ) = delete;
        PresentPass &operator = ( PresentPass && ) = delete;

        ~PresentPass () = default;

        [[nodiscard]] VkResult AcquirePresentTarget ( android_vulkan::Renderer &renderer,
            VkSemaphore acquire
        ) noexcept;

        [[nodiscard]] VkRenderPass GetRenderPass () const noexcept;

        [[nodiscard]] bool OnInitDevice () noexcept;
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

        void InitCommonStructures () noexcept;
};

} // namespace pbr


#endif // PBR_ANDROID_PRESENT_PASS_HPP
