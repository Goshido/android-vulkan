#ifndef PBR_OPAQUE_PROGRAM_HPP
#define PBR_OPAQUE_PROGRAM_HPP


#include "graphics_program.hpp"
#include "resource_heap_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class OpaqueProgram final : public GraphicsProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            VkDeviceAddress                 _transformStream;
            VkDeviceAddress                 _shadingStream;
            VkDeviceAddress                 _frameStream;
            VkDeviceAddress                 _positionStream;
            VkDeviceAddress                 _restStream;
            VkDeviceAddress                 _indexStream;
            uint16_t                        _indexType;
        };

        AV_DX_ALIGNMENT_END

    private:
        ResourceHeapDescriptorSetLayout     _layout {};

    public:
        explicit OpaqueProgram () noexcept;

        OpaqueProgram ( OpaqueProgram const & ) = delete;
        OpaqueProgram &operator = ( OpaqueProgram const & ) = delete;

        OpaqueProgram ( OpaqueProgram && ) = delete;
        OpaqueProgram &operator = ( OpaqueProgram && ) = delete;

        ~OpaqueProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device, VkFormat depthStencilFormat ) noexcept;

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

} // namespace pbr


#endif // PBR_OPAQUE_PROGRAM_HPP
