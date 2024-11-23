#ifndef PBR_OPAQUE_SUBPASS_HPP
#define PBR_OPAQUE_SUBPASS_HPP


#include "default_texture_manager.hpp"
#include "geometry_subpass_base.hpp"
#include "opaque_program.hpp"
#include "sampler_manager.hpp"


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
            GeometryPool &geometryPool,
            MaterialPool &materialPool,
            RenderSessionStats &renderSessionStats,
            VkDescriptorSet samplerDescriptorSet,
            bool &isSamplerUsed
        ) noexcept;

        void UpdateGPUData ( VkCommandBuffer commandBuffer,
            GeometryPool &geometryPool,
            MaterialPool &materialPool,
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
