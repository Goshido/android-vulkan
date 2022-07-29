#ifndef PBR_GEOMETRY_PASS_H
#define PBR_GEOMETRY_PASS_H


#include "geometry_pass_sampler_descriptor_set_layout.h"
#include "opaque_subpass.h"
#include "stipple_subpass.h"


namespace pbr {

class GeometryPass final
{
    private:
        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                             _descriptorSet = VK_NULL_HANDLE;
        GeometryPassSamplerDescriptorSetLayout      _descriptorSetLayout {};

        OpaqueSubpass                               _opaqueSubpass {};
        StippleSubpass                              _stippleSubpass {};

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
            SamplerManager &samplerManager
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        [[nodiscard]] OpaqueSubpass& GetOpaqueSubpass () noexcept;
        [[nodiscard]] StippleSubpass& GetStippleSubpass () noexcept;

        void Reset () noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_H
