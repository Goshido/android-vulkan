#ifndef PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_H
#define PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_PROGRAM_H


#include "program.h"
#include <vulkan_utils.h>
#include "gpgpu_limits.inc"
#include "opaque_instance_descriptor_set_layout.h"


namespace pbr {

class PointLightShadowmapGeneratorProgram final : Program
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
            VkExtent2D const &viewport
        ) override;

        void Destroy ( android_vulkan::Renderer &renderer ) override;
        [[nodiscard]] std::vector<DescriptorSetInfo> const& GetResourceInfo () const override;

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

        [[nodiscard]] bool InitLayout ( VkPipelineLayout &layout,
            android_vulkan::Renderer &renderer
        ) override;

        [[nodiscard]] VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const override;

        [[nodiscard]] VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const override;

        [[nodiscard]] bool InitShaderInfo ( VkPipelineShaderStageCreateInfo const* &targetInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo,
            android_vulkan::Renderer &renderer
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
