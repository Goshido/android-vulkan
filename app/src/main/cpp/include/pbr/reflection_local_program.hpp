#ifndef PBR_REFLECTION_LOCAL_PROGRAM_HPP
#define PBR_REFLECTION_LOCAL_PROGRAM_HPP


#include "light_lightup_base_program.hpp"
#include "lightup_common_descriptor_set_layout.hpp"
#include "light_volume_descriptor_set_layout.hpp"
#include "reflection_local_descriptor_set_layout.hpp"
#include "volume_data.hpp"


namespace pbr {

class ReflectionLocalProgram final : public LightLightupBaseProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

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
        explicit ReflectionLocalProgram () noexcept;

        ReflectionLocalProgram ( ReflectionLocalProgram const & ) = delete;
        ReflectionLocalProgram &operator = ( ReflectionLocalProgram const & ) = delete;

        ReflectionLocalProgram ( ReflectionLocalProgram && ) = delete;
        ReflectionLocalProgram &operator = ( ReflectionLocalProgram && ) = delete;

        ~ReflectionLocalProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            VkExtent2D const &viewport
        ) noexcept;

        void SetLightData ( VkCommandBuffer commandBuffer,
            VkDescriptorSet transform,
            VkDescriptorSet lightData
        ) const noexcept;

    private:
        [[nodiscard]] VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const noexcept override;

        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineDynamicStateCreateInfo const* InitDynamicStateInfo (
            VkPipelineDynamicStateCreateInfo* info
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
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) noexcept override;

        [[nodiscard]] VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D* scissorInfo,
            VkViewport* viewportInfo,
            VkExtent2D const* viewport
        ) const noexcept override;

        [[nodiscard]] VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const noexcept override;
};

} // namespace pbr


#endif // PBR_REFLECTION_LOCAL_PROGRAM_HPP
