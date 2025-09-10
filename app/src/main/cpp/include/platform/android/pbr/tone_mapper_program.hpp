// FUCK - windows and android separation

#ifndef PBR_ANDROID_TONE_MAPPER_PROGRAM_HPP
#define PBR_ANDROID_TONE_MAPPER_PROGRAM_HPP


#include "full_screen_triangle_descriptor_set_layout.hpp"
#include "graphics_program.hpp"
#include <pbr/brightness_info.hpp>
#include "tone_mapper_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::android {

class ToneMapperProgram final : public android::GraphicsProgram
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
        explicit ToneMapperProgram () noexcept;

        ToneMapperProgram ( ToneMapperProgram const & ) = delete;
        ToneMapperProgram &operator = ( ToneMapperProgram const & ) = delete;

        ToneMapperProgram ( ToneMapperProgram && ) = delete;
        ToneMapperProgram &operator = ( ToneMapperProgram && ) = delete;

        ~ToneMapperProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            BrightnessInfo const &brightnessInfo,
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

} // namespace pbr::android


#endif // PBR_ANDROID_TONE_MAPPER_PROGRAM_HPP
