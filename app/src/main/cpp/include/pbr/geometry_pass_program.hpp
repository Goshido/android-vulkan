#ifndef PBR_GEOMETRY_PASS_PROGRAM_HPP
#define PBR_GEOMETRY_PASS_PROGRAM_HPP


#include "geometry_pass_instance_descriptor_set_layout.hpp"
#include "geometry_pass_sampler_descriptor_set_layout.hpp"
#include "geometry_pass_texture_descriptor_set_layout.hpp"
#include "gpgpu_limits.inc"
#include "graphics_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class GeometryPassProgram : public GraphicsProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct InstancePositionData final
        {
            GXMat4                                  _localViewProj[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
        };

        struct TBN64 final
        {
            uint64_t                                _q0;
            uint64_t                                _q1;
        };

        struct InstanceNormalData final
        {
            TBN64                                   _localView[ PBR_OPAQUE_MAX_INSTANCE_COUNT / 2U ];
        };

        class ColorData final
        {
            private:
                uint32_t                            _emiRcol0rgb = 0U;
                uint32_t                            _emiGcol1rgb = 0U;
                uint32_t                            _emiBcol2rgb = 0U;
                uint32_t                            _col0aEmiIntens = 0U;

            public:
                ColorData () = default;

                ColorData ( ColorData const & ) = default;
                ColorData &operator = ( ColorData const & ) = default;

                ColorData ( ColorData && ) = default;
                ColorData &operator = ( ColorData && ) = default;

                ColorData ( GXColorUNORM color0,
                    GXColorUNORM color1,
                    GXColorUNORM color2,
                    GXColorUNORM emission,
                    float emissionIntensity
                ) noexcept;

                ~ColorData () = default;
        };

        struct InstanceColorData final
        {
            ColorData                               _colorData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
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

        [[nodiscard]] DescriptorSetInfo const &GetResourceInfo () const noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) noexcept;

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


#endif // PBR_GEOMETRY_PASS_PROGRAM_HPP
