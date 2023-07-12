#ifndef PBR_OPAQUE_SUBPASS_HPP
#define PBR_OPAQUE_SUBPASS_HPP


#include "default_texture_manager.h"
#include "geometry_subpass_base.h"
#include "opaque_program.h"
#include "sampler_manager.h"


namespace pbr {

class OpaqueSubpass final : public GeometrySubpassBase
{
    private:
        OpaqueProgram       _program {};

    public:
        OpaqueSubpass () = default;

        OpaqueSubpass ( OpaqueSubpass const & ) = delete;
        OpaqueSubpass &operator = ( OpaqueSubpass const & ) = delete;

        OpaqueSubpass ( OpaqueSubpass && ) = delete;
        OpaqueSubpass &operator = ( OpaqueSubpass && ) = delete;

        ~OpaqueSubpass () = default;

        [[nodiscard]] SceneData &GetSceneData () noexcept;

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

        void UpdateGPUData ( VkCommandBuffer commandBuffer,
            MaterialPool &materialPool,
            UniformBufferPoolManager &uniformPool,
            GXProjectionClipPlanes const &frustum,
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


#endif // PBR_OPAQUE_SUBPASS_HPP
