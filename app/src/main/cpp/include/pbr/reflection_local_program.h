#ifndef PBR_REFLECTION_LOCAL_PROGRAM_H
#define PBR_REFLECTION_LOCAL_PROGRAM_H


#include "light_lightup_base_program.h"
#include "lightup_common_descriptor_set_layout.h"
#include "light_volume_descriptor_set_layout.h"
#include "reflection_local_descriptor_set_layout.h"
#include <vulkan_utils.h>


namespace pbr {

class ReflectionLocalProgram final : public LightLightupBaseProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct VolumeData final
        {
            [[maybe_unused]] GXMat4             _transform;
        };

        struct LightData final
        {
            [[maybe_unused]] GXVec3             _locationView;
            [[maybe_unused]] float              _invSize;
        };

        AV_DX_ALIGNMENT_END

    private:
        LightupCommonDescriptorSetLayout        _commonLayout;
        LightVolumeDescriptorSetLayout          _lightVolumeLayout;
        ReflectionLocalDescriptorSetLayout      _reflectionLayout;

    public:
        ReflectionLocalProgram ();

        ReflectionLocalProgram ( ReflectionLocalProgram const & ) = delete;
        ReflectionLocalProgram& operator = ( ReflectionLocalProgram const & ) = delete;

        ReflectionLocalProgram ( ReflectionLocalProgram && ) = delete;
        ReflectionLocalProgram& operator = ( ReflectionLocalProgram && ) = delete;

        ~ReflectionLocalProgram () override = default;

        // Method return true is success. Otherwise method returns false.
        // The method MUST invoke vkCreateGraphicsPipelines at the end.
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) override;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] DescriptorSetInfo const& GetResourceInfo () const override;

        void SetLightData ( VkCommandBuffer commandBuffer, VkDescriptorSet lightData ) const;

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


#endif // PBR_REFLECTION_LOCAL_PROGRAM_H
