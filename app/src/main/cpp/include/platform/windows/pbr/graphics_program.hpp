#ifndef PBR_WINDOWS_GRAPHICS_PROGRAM_HPP
#define PBR_WINDOWS_GRAPHICS_PROGRAM_HPP


#include <pbr/graphics_program.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vector>

GX_RESTORE_WARNING_STATE


// FUCK - remove namespace
namespace pbr::windows {

class GraphicsProgram : public pbr::GraphicsProgram
{
    private:
        uint32_t    _pushConstantSize = 0U;

    public:
        GraphicsProgram () = delete;

        GraphicsProgram ( GraphicsProgram const & ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram const & ) = delete;

        GraphicsProgram ( GraphicsProgram && ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram && ) = delete;

        [[nodiscard]] VkPipelineLayout GetPipelineLayout () const noexcept;
        void SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept;

    protected:
        explicit GraphicsProgram ( std::string_view name, size_t pushConstantSize ) noexcept;
        ~GraphicsProgram () override = default;

        // 'nativeXXX' are needed when format is OS/platform specific and could be known in runtime only.
        // For example swapchain related pipelines or pipelines with depth/stencil features.
        [[nodiscard]] virtual VkPipelineRenderingCreateInfo const* InitRenderingInfo ( VkFormat nativeColor,
            VkFormat nativeDepth,
            VkFormat nativeStencil,
            VkFormat nativeDepthStencil,
            VkFormat* colorAttachments,
            VkPipelineRenderingCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual bool InitShaderInfo ( VkPipelineShaderStageCreateInfo const* &targetInfo,
            std::vector<uint8_t> &vs,
            std::vector<uint8_t> &fs,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkShaderModuleCreateInfo* moduleInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) const noexcept = 0;

        [[nodiscard]] static VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo () noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_GRAPHICS_PROGRAM_HPP
