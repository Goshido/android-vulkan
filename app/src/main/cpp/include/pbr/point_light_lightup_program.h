#ifndef PBR_POINT_LIGHT_LIGHTUP_PROGRAM_H
#define PBR_POINT_LIGHT_LIGHTUP_PROGRAM_H


#include <half_types.h>
#include "lightup_common_descriptor_set_layout.h"
#include "light_lightup_base_program.h"
#include "light_volume_descriptor_set_layout.h"
#include "point_light_descriptor_set_layout.h"


namespace pbr {

class PointLightLightupProgram final : public LightLightupBaseProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct [[maybe_unused]] VolumeData final
        {
            [[maybe_unused]] GXMat4                     _transform;
        };

        struct LightData final
        {
            [[maybe_unused]] GXMat4                     _lightProjection;
            [[maybe_unused]] GXMat4                     _viewToLight;

            [[maybe_unused]] GXVec3                     _lightLocationView;
            [[maybe_unused]] float                      _sceneScaleFactor;

            [[maybe_unused]] android_vulkan::Half3      _hue;
            [[maybe_unused]] android_vulkan::Half       _intensity;

            [[maybe_unused]] GXVec2                     _padding1_0;
        };

        AV_DX_ALIGNMENT_END

    private:
        LightupCommonDescriptorSetLayout                _commonLayout;
        LightVolumeDescriptorSetLayout                  _lightVolumeLayout;
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

        void Destroy ( VkDevice device ) override;

        void SetDescriptorSets ( VkCommandBuffer commandBuffer,
            VkDescriptorSet lightData,
            VkDescriptorSet volumeData
        ) const;

    private:
        [[nodiscard]] DescriptorSetInfo const& GetResourceInfo () const override;

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


#endif // PBR_POINT_LIGHT_LIGHTUP_PROGRAM_H
