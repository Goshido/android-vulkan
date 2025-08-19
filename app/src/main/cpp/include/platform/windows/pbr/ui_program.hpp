#ifndef PBR_WINDOWS_UI_PROGRAM_HPP
#define PBR_WINDOWS_UI_PROGRAM_HPP


#include "graphics_program.hpp"
#include <GXCommon/GXMath.hpp>
#include <pbr/brightness_info.hpp>
#include "resource_heap_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::windows {

class UIProgram final : public windows::GraphicsProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct UIInfo final
        {
            VkDeviceAddress                     _positionBDA;
            VkDeviceAddress                     _restBDA;
            GXVec2                              _rotateScaleRow0;
            GXVec2                              _padding0;
            GXVec2                              _rotateScaleRow1;
            GXVec2                              _offset;
        };

        AV_DX_ALIGNMENT_END

    private:
        ResourceHeapDescriptorSetLayout         _layout {};

    public:
        UIProgram () noexcept;

        UIProgram ( UIProgram const & ) = delete;
        UIProgram &operator = ( UIProgram const & ) = delete;

        UIProgram ( UIProgram && ) = delete;
        UIProgram &operator = ( UIProgram && ) = delete;

        ~UIProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] bool Init ( VkDevice device,
            VkFormat swapchainFormat,
            BrightnessInfo const &brightnessInfo,
            VkExtent2D const &viewport
        ) noexcept;

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

        [[nodiscard]] bool InitShaderInfo ( VkPipelineShaderStageCreateInfo const* &targetInfo,
            std::vector<uint8_t> &vs,
            std::vector<uint8_t> &fs,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkShaderModuleCreateInfo* moduleInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) const noexcept override;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_UI_PROGRAM_HPP
