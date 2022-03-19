#ifndef PBR_REFLECTION_GLOBAL_PROGRAM_H
#define PBR_REFLECTION_GLOBAL_PROGRAM_H


#include "light_lightup_base_program.h"
#include "lightup_common_descriptor_set_layout.h"
#include "stub_descriptor_set_layout.h"
#include "reflection_global_descriptor_set_layout.h"
#include <vulkan_utils.h>


namespace pbr {

class ReflectionGlobalProgram final : public LightLightupBaseProgram
{
    private:
        LightupCommonDescriptorSetLayout        _commonLayout;
        StubDescriptorSetLayout                 _stubLayout;
        ReflectionGlobalDescriptorSetLayout     _reflectionLayout;

    public:
        ReflectionGlobalProgram ();

        ReflectionGlobalProgram ( ReflectionGlobalProgram const & ) = delete;
        ReflectionGlobalProgram& operator = ( ReflectionGlobalProgram const & ) = delete;

        ReflectionGlobalProgram ( ReflectionGlobalProgram && ) = delete;
        ReflectionGlobalProgram& operator = ( ReflectionGlobalProgram && ) = delete;

        ~ReflectionGlobalProgram () override = default;

        // Method return true is success. Otherwise method returns false.
        // The method MUST invoke vkCreateGraphicsPipelines at the end.
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] DescriptorSetInfo const& GetResourceInfo () const noexcept override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

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

        [[nodiscard]] bool InitLayout ( android_vulkan::Renderer &renderer,
            VkPipelineLayout &layout
        ) noexcept override;

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


#endif // PBR_REFLECTION_GLOBAL_PROGRAM_H
