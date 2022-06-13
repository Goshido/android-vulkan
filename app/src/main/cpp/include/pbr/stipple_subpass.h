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
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass renderPass
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager,
            RenderSessionStats &renderSessionStats,
            VkDescriptorSet samplerDescriptorSet,
            bool &isSamplerUsed
        ) noexcept;

    private:
        void ReportGeometry ( RenderSessionStats &renderSessionStats,
            uint32_t vertexCount,
            uint32_t instanceCount
        ) noexcept override;

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager
        ) noexcept;
};

} // namespace pbr


#endif // PBR_STIPPLE_SUBPASS_H
