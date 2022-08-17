#ifndef PBR_STIPPLE_SUBPASS_H
#define PBR_STIPPLE_SUBPASS_H


#include "geometry_subpass_base.h"
#include "default_texture_manager.h"
#include "sampler_manager.h"
#include "stipple_program.h"


namespace pbr {

class StippleSubpass final : public GeometrySubpassBase
{
    private:
        StippleProgram      _program {};

    public:
        StippleSubpass () = default;

        StippleSubpass ( StippleSubpass const & ) = delete;
        StippleSubpass& operator = ( StippleSubpass const & ) = delete;

        StippleSubpass ( StippleSubpass && ) = delete;
        StippleSubpass& operator = ( StippleSubpass && ) = delete;

        ~StippleSubpass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            VkRenderPass renderPass
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        void Execute ( VkCommandBuffer commandBuffer,
            MaterialPool &materialPool,
            UniformBufferPoolManager &uniformPool,
            RenderSessionStats &renderSessionStats,
            VkDescriptorSet samplerDescriptorSet,
            bool &isSamplerUsed
        ) noexcept;

        void UpdateGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            MaterialPool &materialPool,
            UniformBufferPoolManager &uniformPool,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        ) noexcept;

    private:
        void ReportGeometry ( RenderSessionStats &renderSessionStats,
            uint32_t vertexCount,
            uint32_t instanceCount
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_STIPPLE_SUBPASS_H
