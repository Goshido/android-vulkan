#ifndef PBR_OUTLINE_MASK_PROGRAM_HPP
#define PBR_OUTLINE_MASK_PROGRAM_HPP


#include "graphics_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class OutlineMaskProgram final : public GraphicsProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            VkDeviceAddress     _outlineStream;
            VkDeviceAddress     _frameStream;
            VkDeviceAddress     _positionStream;
            VkDeviceAddress     _indexStream;
            uint32_t            _indexType;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit OutlineMaskProgram () noexcept;

        OutlineMaskProgram ( OutlineMaskProgram const & ) = delete;
        OutlineMaskProgram &operator = ( OutlineMaskProgram const & ) = delete;

        OutlineMaskProgram ( OutlineMaskProgram && ) = delete;
        OutlineMaskProgram &operator = ( OutlineMaskProgram && ) = delete;

        ~OutlineMaskProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, VkFormat depthStencilFormat ) noexcept;

    private:
        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineDynamicStateCreateInfo const* InitDynamicStateInfo (
            VkPipelineDynamicStateCreateInfo* info
        ) const noexcept override;

        [[nodiscard]] VkPipelineInputAssemblyStateCreateInfo const* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D* scissorInfo,
            VkViewport* viewportInfo,
            VkExtent2D const* viewport
        ) const noexcept override;

        [[nodiscard]] VkPipelineShaderStageCreateInfo const* InitShaderInfo ( std::vector<uint8_t> &vs,
            std::vector<uint8_t> &fs,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkShaderModuleCreateInfo* moduleInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) const noexcept override;

                [[nodiscard]] VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const noexcept override;

        [[nodiscard]] VkPipelineRenderingCreateInfo const* InitRenderingInfo ( VkFormat nativeColor,
            VkFormat nativeDepth,
            VkFormat nativeStencil,
            VkFormat nativeDepthStencil,
            VkFormat* colorAttachments,
            VkPipelineRenderingCreateInfo &info
        ) const noexcept override;
};

} // namespace pbr


#endif // PBR_OUTLINE_MASK_PROGRAM_HPP
