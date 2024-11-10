#ifndef PBR_GEOMETRY_PASS_HPP
#define PBR_GEOMETRY_PASS_HPP


#include "opaque_subpass.hpp"
#include "stipple_subpass.hpp"


namespace pbr {

class GeometryPass final
{
    private:
        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                             _descriptorSet = VK_NULL_HANDLE;
        GeometryPassSamplerDescriptorSetLayout      _descriptorSetLayout {};

        GeometryPool                                _geometryPool {};
        MaterialPool                                _materialPool {};
        OpaqueSubpass                               _opaqueSubpass {};
        StippleSubpass                              _stippleSubpass {};

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


#endif // PBR_GEOMETRY_PASS_HPP
