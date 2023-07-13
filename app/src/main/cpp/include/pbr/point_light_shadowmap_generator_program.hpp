#ifndef PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_HPP
#define PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_HPP


#include "geometry_pass_instance_descriptor_set_layout.hpp"
#include "gpgpu_limits.inc"
#include "program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class PointLightShadowmapGeneratorProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct ObjectData final
        {
            GXMat4                                  _transform[ PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT ];
        };

        struct InstanceData final
        {
            ObjectData                              _instanceData[ PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT ];
        };

        AV_DX_ALIGNMENT_END

    private:
        GeometryPassInstanceDescriptorSetLayout     _instanceLayout {};

    public:
        PointLightShadowmapGeneratorProgram () noexcept;

        PointLightShadowmapGeneratorProgram ( PointLightShadowmapGeneratorProgram const & ) = delete;
        PointLightShadowmapGeneratorProgram &operator = ( PointLightShadowmapGeneratorProgram const & ) = delete;

        PointLightShadowmapGeneratorProgram ( PointLightShadowmapGeneratorProgram && ) = delete;
        PointLightShadowmapGeneratorProgram &operator = ( PointLightShadowmapGeneratorProgram && ) = delete;

        ~PointLightShadowmapGeneratorProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] DescriptorSetInfo const &GetResourceInfo () const noexcept override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet sets ) const noexcept;

    private:
        [[nodiscard]] VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const noexcept override;

        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineInputAssemblyStateCreateInfo const* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

        [[nodiscard]] VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            VkPipelineShaderStageCreateInfo const* &targetInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) noexcept override;

        void DestroyShaderModules ( VkDevice device ) noexcept override;

        [[nodiscard]] VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D &scissorInfo,
            VkViewport &viewportInfo,
            VkExtent2D const &viewport
        ) const noexcept override;

        [[nodiscard]] VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const noexcept override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_HPP