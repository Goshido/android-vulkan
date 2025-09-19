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

        VkImageMemoryBarrier            _barrier
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_NONE,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = VK_NULL_HANDLE,

            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0U,
                .levelCount = 1U,
                .baseArrayLayer = 0U,
                .layerCount = 1U
            }
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

        VkRenderingAttachmentInfo       _colorAttachment
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

            .clearValue
            {
                .color
                {
                    .float32 { 0.0F, 0.0F, 0.0F, 0.0F }
                }
            }
        };

        VkRenderingInfo                 _renderingInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = 0U,

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

            .layerCount = 1U,
            .viewMask = 0U,
            .colorAttachmentCount = 1U,
            .pColorAttachments = &_colorAttachment,
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr
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

        void Begin ( android_vulkan::Renderer const &renderer, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] std::optional<VkResult> End ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkSemaphore acquire,
            VkFence fence,
            std::mutex* submitMutex
        ) noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_PRESENT_PASS_HPP
