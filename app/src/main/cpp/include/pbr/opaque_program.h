#ifndef OPAQUE_PROGRAM_H
#define OPAQUE_PROGRAM_H


#include <texture2D.h>
#include <vulkan_utils.h>
#include "program.h"
#include "opaque_texture_descriptor_set_layout.h"


namespace pbr {

class OpaqueProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            GXMat4                          _localView;
            GXMat4                          _localViewProjection;
        };

        AV_DX_ALIGNMENT_END

    private:
        OpaqueTextureDescriptorSetLayout    _textureLayout;

    public:
        OpaqueProgram ();

        OpaqueProgram ( OpaqueProgram const &other ) = delete;
        OpaqueProgram& operator = ( OpaqueProgram const &other ) = delete;

        OpaqueProgram ( OpaqueProgram &&other ) = delete;
        OpaqueProgram& operator = ( OpaqueProgram &&other ) = delete;

        ~OpaqueProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            const VkExtent2D &viewport
        ) override;

        void Destroy ( android_vulkan::Renderer &renderer ) override;
        [[nodiscard]] std::vector<DescriptorSetInfo> const& GetResourceInfo () const override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet ) const;
        void SetTransform ( VkCommandBuffer commandBuffer, PushConstants const &transform ) const;

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


#endif // OPAQUE_PROGRAM_H
