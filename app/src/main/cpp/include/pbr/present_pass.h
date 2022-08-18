#ifndef PBR_PRESENT_PASS_H
#define PBR_PRESENT_PASS_H


#include "texture_present_program.h"


namespace pbr {

class PresentPass final
{
    private:
        uint32_t                        _framebufferIndex = std::numeric_limits<uint32_t>::max ();
        std::vector<VkFramebuffer>      _framebuffers {};
        VkRenderPass                    _renderPass = VK_NULL_HANDLE;
        VkPresentInfoKHR                _presentInfo {};
        TexturePresentProgram           _program {};
        VkSemaphore                     _renderEndSemaphore = VK_NULL_HANDLE;
        VkRenderPassBeginInfo           _renderInfo {};
        VkSubmitInfo                    _submitInfo {};
        VkSemaphore                     _targetAcquiredSemaphore = VK_NULL_HANDLE;

    public:
        PresentPass () = default;

        PresentPass ( PresentPass const & ) = delete;
        PresentPass& operator = ( PresentPass const & ) = delete;

        PresentPass ( PresentPass && ) = delete;
        PresentPass& operator = ( PresentPass && ) = delete;

        ~PresentPass () = default;

        [[nodiscard]] bool AcquirePresentTarget ( android_vulkan::Renderer &renderer,
            size_t &swapchainImageIndex
        ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkDescriptorSet presentTarget,
            VkFence fence
        ) noexcept;

    private:
        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyFramebuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void InitCommonStructures ( VkExtent2D const &resolution ) noexcept;
};

} // namespace pbr


#endif // PBR_PRESENT_PASS_H
