#ifndef PBR_LIGHT_VOLUME_PROGRAM_H
#define PBR_LIGHT_VOLUME_PROGRAM_H


#include <vulkan_utils.h>
#include "lightup_common_descriptor_set_layout.h"
#include "light_volume_descriptor_set_layout.h"
#include "program.h"


namespace pbr {

class LightVolumeProgram final : public Program
{
    private:
        constexpr static uint32_t const STENCIL_INITIAL_VALUE = 255U;

    public:
        AV_DX_ALIGNMENT_BEGIN

        struct [[maybe_unused]] VolumeData final
        {
            [[maybe_unused]] GXMat4     _transform;
        };

        AV_DX_ALIGNMENT_END

    private:
        LightupCommonDescriptorSetLayout    _commonLayout;
        LightVolumeDescriptorSetLayout      _lightVolumeLayout;

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
        ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] constexpr static uint32_t GetLightVolumeStencilValue () noexcept
        {
            return STENCIL_INITIAL_VALUE - 1U;
        }

        [[nodiscard]] constexpr static uint32_t GetStencilInitialValue () noexcept
        {
            return STENCIL_INITIAL_VALUE;
        }

        void SetTransform ( VkCommandBuffer commandBuffer, VkDescriptorSet transform ) const noexcept;

    private:
        [[nodiscard]] DescriptorSetInfo const& GetResourceInfo () const noexcept override;

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

        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

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


#endif // PBR_LIGHT_VOLUME_PROGRAM_H
