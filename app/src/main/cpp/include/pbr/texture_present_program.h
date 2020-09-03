#ifndef TEXTURE_PRESENT_PROGRAM_H
#define TEXTURE_PRESENT_PROGRAM_H


#include <texture2D.h>
#include <vulkan_utils.h>
#include "program.h"


namespace pbr {

class TexturePresentProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            GXMat4                  _transform;
        };

        AV_DX_ALIGNMENT_END

    public:
        TexturePresentProgram ();
        ~TexturePresentProgram () override = default;

        TexturePresentProgram ( const TexturePresentProgram &other ) = delete;
        TexturePresentProgram& operator = ( const TexturePresentProgram &other ) = delete;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            const VkExtent2D &viewport
        ) override;

        void Destroy ( android_vulkan::Renderer &renderer ) override;
        const std::vector<ProgramResource>& GetResourceInfo () const override;

    private:
        void BeginSetup () override;
        void EndSetup () override;

        bool Bind ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] const VkPipelineColorBlendStateCreateInfo* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const override;

        [[nodiscard]] const VkPipelineDepthStencilStateCreateInfo* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const override;

        [[nodiscard]] const VkPipelineInputAssemblyStateCreateInfo* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const override;

        [[nodiscard]] bool InitLayout ( VkPipelineLayout &layout, android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] const VkPipelineMultisampleStateCreateInfo* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const override;

        [[nodiscard]] const VkPipelineRasterizationStateCreateInfo* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const override;

        [[nodiscard]] bool InitShaderInfo ( const VkPipelineShaderStageCreateInfo* &targetInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo,
            android_vulkan::Renderer &renderer
        ) override;

        [[nodiscard]] const VkPipelineViewportStateCreateInfo* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D &scissorInfo,
            VkViewport &viewportInfo,
            const VkExtent2D &viewport
        ) const override;

        [[nodiscard]] const VkPipelineVertexInputStateCreateInfo* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const override;
};


} // namespace pbr


#endif // TEXTURE_PRESENT_PROGRAM_H