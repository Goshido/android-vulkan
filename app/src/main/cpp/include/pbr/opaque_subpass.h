#ifndef PBR_OPAQUE_SUBPASS_H
#define PBR_OPAQUE_SUBPASS_H


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
        OpaqueSubpass& operator = ( OpaqueSubpass const & ) = delete;

        OpaqueSubpass ( OpaqueSubpass && ) = delete;
        OpaqueSubpass& operator = ( OpaqueSubpass && ) = delete;

        ~OpaqueSubpass () = default;

        [[nodiscard]] SceneData& GetSceneData () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass renderPass
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager,
            SamplerManager &samplerManager,
            RenderSessionStats &renderSessionStats
        ) noexcept;

    private:
        void ReportGeometry ( RenderSessionStats &renderSessionStats,
            uint32_t vertexCount,
            uint32_t instanceCount
        ) noexcept override;

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager,
            SamplerManager &samplerManager
        ) noexcept;
};

} // namespace pbr


#endif // PBR_OPAQUE_SUBPASS_H
