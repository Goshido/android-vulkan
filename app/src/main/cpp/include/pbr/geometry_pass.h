#ifndef PBR_GEOMETRY_PASS_H
#define PBR_GEOMETRY_PASS_H


#include <renderer.h>
#include "geometry_pass_sampler_descriptor_set_layout.h"
#include "opaque_subpass.h"
#include "stipple_subpass.h"


namespace pbr {

class GeometryPass final
{
    private:
        VkCommandBuffer                             _commandBuffer = VK_NULL_HANDLE;

        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                             _descriptorSet = VK_NULL_HANDLE;
        GeometryPassSamplerDescriptorSetLayout      _descriptorSetLayout {};

        VkFence                                     _fence = VK_NULL_HANDLE;

        OpaqueSubpass                               _opaqueSubpass {};
        StippleSubpass                              _stippleSubpass {};

        VkRenderPassBeginInfo                       _renderPassInfo {};

    public:
        GeometryPass () = default;

        GeometryPass ( GeometryPass const & ) = delete;
        GeometryPass& operator = ( GeometryPass const & ) = delete;

        GeometryPass ( GeometryPass && ) = delete;
        GeometryPass& operator = ( GeometryPass && ) = delete;

        ~GeometryPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass renderPass,
            VkFramebuffer framebuffer,
            SamplerManager &samplerManager
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        // The method returns command buffer in recording state if success. Otherwise the method returns VK_NULL_HANDLE.
        [[nodiscard]] VkCommandBuffer Execute ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        [[nodiscard]] VkFence GetFence () const noexcept;
        [[nodiscard]] OpaqueSubpass& GetOpaqueSubpass () noexcept;
        [[nodiscard]] StippleSubpass& GetStippleSubpass () noexcept;

        void Reset () noexcept;
        [[nodiscard]] bool WaitReady ( android_vulkan::Renderer &renderer ) const noexcept;

        // Return value optimization friendly method.
        [[nodiscard]] static VkSubpassDescription GetSubpassDescription () noexcept;

    private:
        [[nodiscard]] bool BeginRenderPass () noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_H
