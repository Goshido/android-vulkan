#ifndef PBR_POINT_LIGHT_LIGHTUP_PROGRAM_H
#define PBR_POINT_LIGHT_LIGHTUP_PROGRAM_H


#include <half_types.h>
#include <vulkan_utils.h>
#include "lightup_common_descriptror_set_layout.h"
#include "point_light_descriptor_set_layout.h"
#include "program.h"


namespace pbr {

class PointLightLightupProgram final : Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct [[maybe_unused]] PushConstants final
        {
            [[maybe_unused]] GXMat4                     _transform;
        };

        struct [[maybe_unused]] ViewData final
        {
            [[maybe_unused]] GXMat4                     _toView;
            [[maybe_unused]] GXVec2                     _invResolutionFactor;
            [[maybe_unused]] GXVec2                     _padding0_0;
        };

        struct [[maybe_unused]] LightData final
        {
            [[maybe_unused]] GXMat4                     _projection;
            [[maybe_unused]] GXMat4                     _shadowmapTransform;

            [[maybe_unused]] GXVec3                     _lightLocationView;
            [[maybe_unused]] float                      _padding1_0;

            [[maybe_unused]] android_vulkan::Half3      _hue;
            [[maybe_unused]] android_vulkan::Half       _intensity;

            [[maybe_unused]] android_vulkan::Half3      _toLightDirectionView;
            [[maybe_unused]] android_vulkan::Half       _padding1_1;
        };

        AV_DX_ALIGNMENT_END

    private:
        LightupCommonDescriptorSetLayout                _commonLayout;
        PointLightDescriptorSetLayout                   _pointLightLayout;

    public:
        PointLightLightupProgram () noexcept;

        PointLightLightupProgram ( PointLightLightupProgram const & ) = delete;
        PointLightLightupProgram& operator = ( PointLightLightupProgram const & ) = delete;

        PointLightLightupProgram ( PointLightLightupProgram && ) = delete;
        PointLightLightupProgram& operator = ( PointLightLightupProgram && ) = delete;

        ~PointLightLightupProgram () = default;

        // Method return true is success. Otherwise method returns false.
        // The method MUST invoke vkCreateGraphicsPipelines at the end.
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) override;

        void Destroy ( android_vulkan::Renderer &renderer ) override;

    private:
        [[nodiscard]] std::vector<DescriptorSetInfo> const& GetResourceInfo () const override;

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

        [[nodiscard]] bool InitLayout ( VkPipelineLayout &layout, android_vulkan::Renderer &renderer ) override;

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


#endif // PBR_POINT_LIGHT_LIGHTUP_PROGRAM_H
