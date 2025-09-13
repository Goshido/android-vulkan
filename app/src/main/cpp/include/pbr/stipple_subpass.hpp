#ifndef PBR_STIPPLE_SUBPASS_HPP
#define PBR_STIPPLE_SUBPASS_HPP


#include "geometry_subpass_base.hpp"
#include "default_texture_manager.hpp"
#include "sampler_manager.hpp"
#include "stipple_program.hpp"


namespace pbr {

class StippleSubpass final : public GeometrySubpassBase
{
    private:
        StippleProgram      _program {};

    public:
        explicit StippleSubpass () = default;

        StippleSubpass ( StippleSubpass const & ) = delete;
        StippleSubpass &operator = ( StippleSubpass const & ) = delete;

        StippleSubpass ( StippleSubpass && ) = delete;
        StippleSubpass &operator = ( StippleSubpass && ) = delete;

        ~StippleSubpass () = default;

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

        void UpdateGPUData ( GeometryPool &geometryPool,
            MaterialPool &materialPool,
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


#endif // PBR_STIPPLE_SUBPASS_HPP
