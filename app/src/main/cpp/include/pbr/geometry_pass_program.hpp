#ifndef PBR_GEOMETRY_PASS_PROGRAM_HPP
#define PBR_GEOMETRY_PASS_PROGRAM_HPP


#include "geometry_pass_instance_descriptor_set_layout.h"
#include "geometry_pass_sampler_descriptor_set_layout.h"
#include "geometry_pass_texture_descriptor_set_layout.h"
#include "gpgpu_limits.inc"
#include "program.h"
#include <vulkan_utils.h>


namespace pbr {

class GeometryPassProgram : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct ObjectData final
        {
            GXMat4                                  _localView;
            GXMat4                                  _localViewProjection;
            [[maybe_unused]] GXColorRGB             _color0;
            [[maybe_unused]] GXColorRGB             _color1;
            [[maybe_unused]] GXColorRGB             _color2;
            [[maybe_unused]] GXColorRGB             _emission;
        };

        struct InstanceData final
        {
            ObjectData                              _instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
        };

        AV_DX_ALIGNMENT_END

    private:
        GeometryPassInstanceDescriptorSetLayout     _instanceLayout {};
        GeometryPassSamplerDescriptorSetLayout      _samplerLayout {};
        GeometryPassTextureDescriptorSetLayout      _textureLayout {};
        std::string_view const                      _fragmentShaderSource;

    public:
        GeometryPassProgram () = delete;

        GeometryPassProgram ( GeometryPassProgram const & ) = delete;
        GeometryPassProgram &operator = ( GeometryPassProgram const & ) = delete;

        GeometryPassProgram ( GeometryPassProgram && ) = delete;
        GeometryPassProgram &operator = ( GeometryPassProgram && ) = delete;

        ~GeometryPassProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] DescriptorSetInfo const &GetResourceInfo () const noexcept override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer,
            VkDescriptorSet const* sets,
            uint32_t startIndex,
            uint32_t count
        ) const noexcept;

    protected:
        explicit GeometryPassProgram ( std::string_view &&name, std::string_view &&fragmentShader ) noexcept;

    private:
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


#endif // PBR_GEOMETRY_PASS_PROGRAM_HPP
