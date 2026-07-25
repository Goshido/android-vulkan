#ifndef PBR_GIZMO_PREPASS_PROGRAM_HPP
#define PBR_GIZMO_PREPASS_PROGRAM_HPP


#include "graphics_program.hpp"
#include <GXCommon/GXMath.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

class GizmoPrepassProgram final : public GraphicsProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] GXMat4             _toCVV;
            [[maybe_unused]] GXVec3             _cameraPositionWorld;
            [[maybe_unused]] float              _maxRayDistance;
            [[maybe_unused]] GXVec3             _viWorld;
            [[maybe_unused]] float              _invMaxRayDistanceFactor;
            [[maybe_unused]] uint32_t           _tileCountWidth;
            [[maybe_unused]] uint32_t           _tileCounters;
            [[maybe_unused]] VkDeviceAddress    _palette;
            [[maybe_unused]] VkDeviceAddress    _tileSamples;
            [[maybe_unused]] VkDeviceAddress    _vertexStream;
            [[maybe_unused]] VkDeviceAddress    _pixelStream;
            [[maybe_unused]] VkDeviceAddress    _shapeStream;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit GizmoPrepassProgram () noexcept;

        GizmoPrepassProgram ( GizmoPrepassProgram const & ) = delete;
        GizmoPrepassProgram &operator = ( GizmoPrepassProgram const & ) = delete;

        GizmoPrepassProgram ( GizmoPrepassProgram && ) = delete;
        GizmoPrepassProgram &operator = ( GizmoPrepassProgram && ) = delete;

        ~GizmoPrepassProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( VkDevice device, VkFormat swapchainFormat, VkFormat depthStencilFormat ) noexcept;

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

        [[nodiscard]] VkPipelineRenderingCreateInfo const* InitRenderingInfo ( VkFormat nativeColor,
            VkFormat nativeDepth,
            VkFormat nativeStencil,
            VkFormat nativeDepthStencil,
            VkFormat* colorAttachments,
            VkPipelineRenderingCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineShaderStageCreateInfo const* InitShaderInfo ( std::vector<uint8_t> &vs,
            std::vector<uint8_t> &fs,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkShaderModuleCreateInfo* moduleInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) const noexcept override;
};

} // namespace pbr


#endif // PBR_GIZMO_PREPASS_PROGRAM_HPP
