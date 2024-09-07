#ifndef PBR_TONE_MAPPER_PROGRAM_HPP
#define PBR_TONE_MAPPER_PROGRAM_HPP


#include "full_screen_triangle_descriptor_set_layout.hpp"
#include "srgb_program.hpp"
#include "tone_mapper_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class ToneMapperProgram final : public SRGBProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct Transform final
        {
            GXVec2                                  _transformRow0;
            GXVec2                                  _padding0;
            GXVec2                                  _transformRow1;
        };

        AV_DX_ALIGNMENT_END

    private:
        FullScreenTriangleDescriptorSetLayout       _fullScreenTriangleLayout {};
        ToneMapperDescriptorSetLayout               _toneMapperLayout {};

    public:
        ToneMapperProgram () noexcept;

        ToneMapperProgram ( ToneMapperProgram const & ) = delete;
        ToneMapperProgram &operator = ( ToneMapperProgram const & ) = delete;

        ToneMapperProgram ( ToneMapperProgram && ) = delete;
        ToneMapperProgram &operator = ( ToneMapperProgram && ) = delete;

        ~ToneMapperProgram () override = default;

        [[nodiscard]] DescriptorSetInfo const &GetResourceInfo () const noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            SpecializationData specializationData,
            VkExtent2D const &viewport
        ) noexcept;

        void SetDescriptorSets ( VkCommandBuffer commandBuffer, VkDescriptorSet const* sets ) const noexcept;

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


#endif // PBR_TONE_MAPPER_PROGRAM_HPP
