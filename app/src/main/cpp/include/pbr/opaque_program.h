#ifndef OPAQUE_PROGRAM_H
#define OPAQUE_PROGRAM_H


#include <texture2D.h>
#include <vulkan_utils.h>
#include "program.h"


namespace pbr {

class OpaqueProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            GXMat4                      _localView;
            GXMat4                      _localViewProjection;
        };

        AV_DX_ALIGNMENT_END

    private:
        android_vulkan::Texture2D*      _albedoTexture;
        VkSampler                       _albedoSampler;

        android_vulkan::Texture2D*      _emissionTexture;
        VkSampler                       _emissionSampler;

        android_vulkan::Texture2D*      _normalTexture;
        VkSampler                       _normalSampler;

        android_vulkan::Texture2D*      _paramTexture;
        VkSampler                       _paramSampler;

    public:
        OpaqueProgram ();
        ~OpaqueProgram () override = default;

        OpaqueProgram ( const OpaqueProgram &other ) = delete;
        OpaqueProgram& operator = ( const OpaqueProgram &other ) = delete;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            const VkExtent2D &viewport
        ) override;

        void Destroy ( android_vulkan::Renderer &renderer ) override;
        const std::vector<ProgramResource>& GetResourceInfo () const override;

        [[maybe_unused]] void SetAlbedo ( android_vulkan::Texture2D &texture, VkSampler sampler );
        [[maybe_unused]] void SetEmission ( android_vulkan::Texture2D &texture, VkSampler sampler );
        [[maybe_unused]] void SetNormal ( android_vulkan::Texture2D &texture, VkSampler sampler );
        [[maybe_unused]] void SetParams ( android_vulkan::Texture2D &texture, VkSampler sampler );

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


#endif // OPAQUE_PROGRAM_H
