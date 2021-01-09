#ifndef PBR_PRESENT_PASS_H
#define PBR_PRESENT_PASS_H


#include "texture_present_program.h"


namespace pbr {

class PresentPass final
{
    private:
        uint32_t                        _framebufferIndex;
        std::vector<VkFramebuffer>      _framebuffers;
        VkRenderPass                    _renderPass;
        VkPresentInfoKHR                _presentInfo;
        TexturePresentProgram           _program;
        VkSemaphore                     _renderEndSemaphore;
        VkRenderPassBeginInfo           _renderInfo;
        VkSubmitInfo                    _submitInfo;
        VkSemaphore                     _targetAcquiredSemaphore;

    public:
        PresentPass () noexcept;

        PresentPass ( PresentPass const & ) = delete;
        PresentPass& operator = ( PresentPass const & ) = delete;

        PresentPass ( PresentPass && ) = delete;
        PresentPass& operator = ( PresentPass && ) = delete;

        ~PresentPass () = default;

        [[nodiscard]] bool AcquirePresentTarget ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
        void Destroy ( VkDevice device );

        [[nodiscard]] bool Execute ( VkCommandBuffer commandBuffer,
            VkDescriptorSet presentTarget,
            VkFence fence,
            android_vulkan::Renderer &renderer
        );

    private:
        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( VkDevice device );

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void InitCommonStructures ( VkExtent2D const &resolution );
};

} // namespace pbr


#endif // PBR_PRESENT_PASS_H
