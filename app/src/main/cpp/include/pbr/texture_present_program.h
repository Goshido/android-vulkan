#ifndef TEXTURE_PRESENT_PROGRAM_H
#define TEXTURE_PRESENT_PROGRAM_H


#include <texture2D.h>
#include <vulkan_utils.h>
#include "program.h"
#include "texture_present_descriptor_set_layout.h"


namespace pbr {

class TexturePresentProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            GXMat4                              _transform;
        };

        AV_DX_ALIGNMENT_END

    private:
        TexturePresentDescriptorSetLayout       _descriptorSetLayout;

    public:
        TexturePresentProgram () noexcept;

        TexturePresentProgram ( TexturePresentProgram const & ) = delete;
        TexturePresentProgram& operator = ( TexturePresentProgram const & ) = delete;

        TexturePresentProgram ( TexturePresentProgram && ) = delete;
        TexturePresentProgram& operator = ( TexturePresentProgram && ) = delete;

        ~TexturePresentProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) override;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] std::vector<DescriptorSetInfo> const& GetResourceInfo () const override;

        void SetData ( VkCommandBuffer commandBuffer, VkDescriptorSet set, GXMat4 const &transform ) const;

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


#endif // TEXTURE_PRESENT_PROGRAM_H
