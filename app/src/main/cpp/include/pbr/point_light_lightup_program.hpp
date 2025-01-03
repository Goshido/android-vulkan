#ifndef PBR_POINT_LIGHT_LIGHTUP_PROGRAM_HPP
#define PBR_POINT_LIGHT_LIGHTUP_PROGRAM_HPP


#include "lightup_common_descriptor_set_layout.hpp"
#include "light_lightup_base_program.hpp"
#include "light_volume_descriptor_set_layout.hpp"
#include "point_light_descriptor_set_layout.hpp"
#include "volume_data.hpp"


namespace pbr {

class PointLightLightupProgram final : public LightLightupBaseProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct LightData final
        {
            [[maybe_unused]] GXMat4         _lightProjection;
            [[maybe_unused]] GXMat4         _viewToLight;

            [[maybe_unused]] GXVec3         _lightLocationView;
            [[maybe_unused]] float          _sceneScaleFactor;

            [[maybe_unused]] GXVec3         _hue;
            [[maybe_unused]] float          _intensity;
        };

        AV_DX_ALIGNMENT_END

    private:
        LightupCommonDescriptorSetLayout    _commonLayout {};
        LightVolumeDescriptorSetLayout      _lightVolumeLayout {};
        PointLightDescriptorSetLayout       _pointLightLayout {};

    public:
        explicit PointLightLightupProgram () noexcept;

        PointLightLightupProgram ( PointLightLightupProgram const & ) = delete;
        PointLightLightupProgram &operator = ( PointLightLightupProgram const & ) = delete;

        PointLightLightupProgram ( PointLightLightupProgram && ) = delete;
        PointLightLightupProgram &operator = ( PointLightLightupProgram && ) = delete;

        ~PointLightLightupProgram () = default;

        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            VkExtent2D const &viewport
        ) noexcept;

        void SetLightData ( VkCommandBuffer commandBuffer,
            VkDescriptorSet transform,
            VkDescriptorSet lightData
        ) const noexcept;

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


#endif // PBR_POINT_LIGHT_LIGHTUP_PROGRAM_HPP
