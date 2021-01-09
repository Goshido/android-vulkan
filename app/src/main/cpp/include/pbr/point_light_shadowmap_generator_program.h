#ifndef PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_H
#define PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_H


#include "program.h"
#include <vulkan_utils.h>
#include "gpgpu_limits.inc"
#include "opaque_instance_descriptor_set_layout.h"


namespace pbr {

class PointLightShadowmapGeneratorProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct ObjectData final
        {
            [[maybe_unused]] GXMat4             _transform[ PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT ];
        };

        struct [[maybe_unused]] InstanceData final
        {
            [[maybe_unused]] ObjectData         _instanceData[ PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT ];
        };

        AV_DX_ALIGNMENT_END

    private:
        OpaqueInstanceDescriptorSetLayout       _instanceLayout;

    public:
        PointLightShadowmapGeneratorProgram () noexcept;

        PointLightShadowmapGeneratorProgram ( PointLightShadowmapGeneratorProgram const & ) = delete;
        PointLightShadowmapGeneratorProgram& operator = ( PointLightShadowmapGeneratorProgram const & ) = delete;

        PointLightShadowmapGeneratorProgram ( PointLightShadowmapGeneratorProgram && ) = delete;
        PointLightShadowmapGeneratorProgram& operator = ( PointLightShadowmapGeneratorProgram && ) = delete;

        ~PointLightShadowmapGeneratorProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) override;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] std::vector<DescriptorSetInfo> const& GetResourceInfo () const override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet sets ) const;

    private:
        [[nodiscard]] VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const override;

        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const override;

        [[nodiscard]] VkPipelineInputAssemblyStateCreateInfo const* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const override;

        [[nodiscard]] bool InitLayout ( android_vulkan::Renderer &renderer, VkPipelineLayout &layout ) override;

        [[nodiscard]] VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const override;

        [[nodiscard]] VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const override;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            VkPipelineShaderStageCreateInfo const* &targetInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) override;

        [[nodiscard]] VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D &scissorInfo,
            VkViewport &viewportInfo,
            VkExtent2D const &viewport
        ) const override;

        [[nodiscard]] VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_H
