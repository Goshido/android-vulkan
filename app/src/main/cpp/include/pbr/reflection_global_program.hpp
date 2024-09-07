#ifndef PBR_REFLECTION_GLOBAL_PROGRAM_HPP
#define PBR_REFLECTION_GLOBAL_PROGRAM_HPP


#include "light_lightup_base_program.hpp"
#include "lightup_common_descriptor_set_layout.hpp"
#include "stub_descriptor_set_layout.hpp"
#include "reflection_global_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class ReflectionGlobalProgram final : public LightLightupBaseProgram
{
    private:
        LightupCommonDescriptorSetLayout        _commonLayout {};
        StubDescriptorSetLayout                 _stubLayout {};
        ReflectionGlobalDescriptorSetLayout     _reflectionLayout {};

    public:
        ReflectionGlobalProgram () noexcept;

        ReflectionGlobalProgram ( ReflectionGlobalProgram const & ) = delete;
        ReflectionGlobalProgram &operator = ( ReflectionGlobalProgram const & ) = delete;

        ReflectionGlobalProgram ( ReflectionGlobalProgram && ) = delete;
        ReflectionGlobalProgram &operator = ( ReflectionGlobalProgram && ) = delete;

        ~ReflectionGlobalProgram () override = default;

        [[nodiscard]] DescriptorSetInfo const &GetResourceInfo () const noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) noexcept;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

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

        void DestroyShaderModules ( VkDevice device ) noexcept override;

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


#endif // PBR_REFLECTION_GLOBAL_PROGRAM_HPP
