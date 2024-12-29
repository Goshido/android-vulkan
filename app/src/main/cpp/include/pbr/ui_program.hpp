#ifndef PBR_UI_PROGRAM_HPP
#define PBR_UI_PROGRAM_HPP


#include "brightness_program.hpp"
#include "ui_pass_common_descriptor_set_layout.hpp"
#include "ui_pass_image_descriptor_set_layout.hpp"
#include "ui_pass_transform_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class UIProgram final : public BrightnessProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct Transform final
        {
            GXVec2                              _rotateScaleRow0;
            GXVec2                              _padding0;
            GXVec2                              _rotateScaleRow1;
            GXVec2                              _offset;
        };

        AV_DX_ALIGNMENT_END

    private:
        UIPassCommonDescriptorSetLayout         _commonLayout {};
        UIPassImageDescriptorSetLayout          _imageLayout {};
        UIPassTransformDescriptorSetLayout      _transformLayout {};

    public:
        UIProgram () noexcept;

        UIProgram ( UIProgram const & ) = delete;
        UIProgram &operator = ( UIProgram const & ) = delete;

        UIProgram ( UIProgram && ) = delete;
        UIProgram &operator = ( UIProgram && ) = delete;

        ~UIProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            BrightnessInfo const &brightnessInfo,
            VkExtent2D const &viewport
        ) noexcept;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer,
            VkDescriptorSet const* sets,
            uint32_t startIndex,
            uint32_t count
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


#endif // PBR_UI_PROGRAM_HPP
