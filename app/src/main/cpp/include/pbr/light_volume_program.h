#ifndef PBR_LIGHT_VOLUME_PROGRAM_H
#define PBR_LIGHT_VOLUME_PROGRAM_H


#include <vulkan_utils.h>
#include "program.h"


namespace pbr {

class LightVolumeProgram final : public Program
{
    private:
        constexpr static uint32_t const STENCIL_INITIAL_VALUE = 255U;

    public:
        AV_DX_ALIGNMENT_BEGIN

        struct [[maybe_unused]] PushConstants final
        {
            [[maybe_unused]] GXMat4     _transform;
        };

        AV_DX_ALIGNMENT_END

    public:
        LightVolumeProgram () noexcept;

        LightVolumeProgram ( LightVolumeProgram const & ) = delete;
        LightVolumeProgram& operator = ( LightVolumeProgram const & ) = delete;

        LightVolumeProgram ( LightVolumeProgram && ) = delete;
        LightVolumeProgram& operator = ( LightVolumeProgram && ) = delete;

        ~LightVolumeProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) override;

        void Destroy ( VkDevice device ) override;

        [[maybe_unused]] [[nodiscard]] constexpr static uint32_t GetLightVolumeStencilValue ()
        {
            return STENCIL_INITIAL_VALUE - 1U;
        }

        [[maybe_unused]] [[nodiscard]] constexpr static uint32_t GetStencilInitialValue ()
        {
            return STENCIL_INITIAL_VALUE;
        }

    private:
        [[nodiscard]] std::vector<DescriptorSetInfo> const& GetResourceInfo () const override;

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


#endif // PBR_LIGHT_VOLUME_PROGRAM_H
