#ifndef PBR_OPAQUE_PROGRAM_H
#define PBR_OPAQUE_PROGRAM_H


#include "program.h"
#include <vulkan_utils.h>
#include "gpgpu_limits.inc"
#include "opaque_instance_descriptor_set_layout.h"
#include "opaque_texture_descriptor_set_layout.h"


namespace pbr {

class OpaqueProgram final : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct ObjectData final
        {
            GXMat4                              _localView;
            GXMat4                              _localViewProjection;
            GXColorRGB                          _color0;
            GXColorRGB                          _color1;
            GXColorRGB                          _color2;
            GXColorRGB                          _color3;
        };

        struct InstanceData final
        {
             ObjectData                         _instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
        };

        AV_DX_ALIGNMENT_END

    private:
        OpaqueInstanceDescriptorSetLayout       _instanceLayout;
        OpaqueTextureDescriptorSetLayout        _textureLayout;

    public:
        OpaqueProgram ();
        OpaqueProgram ( OpaqueProgram const &other ) = delete;
        OpaqueProgram& operator = ( OpaqueProgram const &other ) = delete;

        OpaqueProgram ( OpaqueProgram &&other ) = delete;
        OpaqueProgram& operator = ( OpaqueProgram &&other ) = delete;

        ~OpaqueProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            VkExtent2D const &viewport
        ) override;

        void Destroy ( android_vulkan::Renderer &renderer ) override;
        [[nodiscard]] std::vector<DescriptorSetInfo> const& GetResourceInfo () const override;

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

        [[nodiscard]] bool InitLayout ( VkPipelineLayout &layout,
            android_vulkan::Renderer &renderer
        ) override;

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


#endif // PBR_OPAQUE_PROGRAM_H
