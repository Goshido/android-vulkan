#ifndef PBR_GRAPHICS_PROGRAM_HPP
#define PBR_GRAPHICS_PROGRAM_HPP


#include <pbr/graphics_program_base.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vector>

GX_RESTORE_WARNING_STATE


namespace pbr {

class GraphicsProgram : public GraphicsProgramBase
{
    public:
        GraphicsProgram () = delete;

        GraphicsProgram ( GraphicsProgram const & ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram const & ) = delete;

        GraphicsProgram ( GraphicsProgram && ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram && ) = delete;

        void SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept;

        ~GraphicsProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;

    protected:
        explicit GraphicsProgram ( size_t pushConstantSize ) noexcept;

        // 'nativeXXX' are needed when format is OS/platform specific and could be known in runtime only.
        // For example swapchain related pipelines or pipelines with depth/stencil features.
        [[nodiscard]] virtual VkPipelineRenderingCreateInfo const* InitRenderingInfo ( VkFormat nativeColor,
            VkFormat nativeDepth,
            VkFormat nativeStencil,
            VkFormat nativeDepthStencil,
            VkFormat* colorAttachments,
            VkPipelineRenderingCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineShaderStageCreateInfo const* InitShaderInfo ( std::vector<uint8_t> &vs,
            std::vector<uint8_t> &fs,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkShaderModuleCreateInfo* moduleInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) const noexcept = 0;

        [[nodiscard]] static VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo () noexcept;
};

} // namespace pbr


#endif // PBR_GRAPHICS_PROGRAM_HPP
