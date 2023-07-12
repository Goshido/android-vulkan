#ifndef PBR_GEOMETRY_PASS_H
#define PBR_GEOMETRY_PASS_H


#include "opaque_subpass.h"
#include "stipple_subpass.h"


namespace pbr {

class GeometryPass final
{
    private:
        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                             _descriptorSet = VK_NULL_HANDLE;
        GeometryPassSamplerDescriptorSetLayout      _descriptorSetLayout {};

        MaterialPool                                _materialPool {};
        OpaqueSubpass                               _opaqueSubpass {};
        StippleSubpass                              _stippleSubpass {};

        UniformBufferPoolManager                    _uniformPool
        {
            eUniformPoolSize::Huge_64M,
            AV_VK_FLAG ( VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) | AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT )
        };

    public:
        GeometryPass () = default;

        GeometryPass ( GeometryPass const & ) = delete;
        GeometryPass &operator = ( GeometryPass const & ) = delete;

        GeometryPass ( GeometryPass && ) = delete;
        GeometryPass &operator = ( GeometryPass && ) = delete;

        ~GeometryPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            VkRenderPass renderPass,
            SamplerManager &samplerManager,
            DefaultTextureManager const &defaultTextureManager
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        void Execute ( VkCommandBuffer commandBuffer, RenderSessionStats &renderSessionStats ) noexcept;

        [[nodiscard]] OpaqueSubpass &GetOpaqueSubpass () noexcept;
        [[nodiscard]] StippleSubpass &GetStippleSubpass () noexcept;

        void Reset () noexcept;

        void UploadGPUData ( VkDevice device,
            VkCommandBuffer commandBuffer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_H
