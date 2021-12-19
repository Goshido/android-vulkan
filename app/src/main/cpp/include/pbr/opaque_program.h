#ifndef PBR_OPAQUE_PROGRAM_H
#define PBR_OPAQUE_PROGRAM_H


#include "gpgpu_limits.inc"
#include "opaque_instance_descriptor_set_layout.h"
#include "opaque_texture_descriptor_set_layout.h"
#include "program.h"
#include <vulkan_utils.h>


namespace pbr {

class OpaqueProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct ObjectData final
        {
            GXMat4                              _localView;
            GXMat4                              _localViewProjection;
            [[maybe_unused]] GXColorRGB         _color0;
            [[maybe_unused]] GXColorRGB         _color1;
            [[maybe_unused]] GXColorRGB         _color2;
            [[maybe_unused]] GXColorRGB         _color3;
        };

        struct InstanceData final
        {
            ObjectData                          _instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
        };

        AV_DX_ALIGNMENT_END

    private:
        OpaqueInstanceDescriptorSetLayout       _instanceLayout;
        OpaqueTextureDescriptorSetLayout        _textureLayout;

    public:
        OpaqueProgram () noexcept;

        OpaqueProgram ( OpaqueProgram const & ) = delete;
        OpaqueProgram& operator = ( OpaqueProgram const & ) = delete;

        OpaqueProgram ( OpaqueProgram && ) = delete;
        OpaqueProgram& operator = ( OpaqueProgram && ) = delete;

        ~OpaqueProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) override;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] DescriptorSetInfo const& GetResourceInfo () const override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer,
            VkDescriptorSet const* sets,
            uint32_t startIndex,
            uint32_t count
        ) const;

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


#endif // PBR_OPAQUE_PROGRAM_H
