#ifndef PBR_GBUFFER_PROGRAM_HPP
#define PBR_GBUFFER_PROGRAM_HPP


#include "graphics_program.hpp"
#include "resource_heap_descriptor_set_layout.hpp"


namespace pbr {

class GBufferProgram : public GraphicsProgram
{
    private:
        std::string_view                    _vsSource {};
        std::string_view                    _fsSource {};
        ResourceHeapDescriptorSetLayout     _layout {};

    public:
        GBufferProgram () = delete;

        GBufferProgram ( GBufferProgram const & ) = delete;
        GBufferProgram &operator = ( GBufferProgram const & ) = delete;

        GBufferProgram ( GBufferProgram && ) = delete;
        GBufferProgram &operator = ( GBufferProgram && ) = delete;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device, VkFormat depthStencilFormat ) noexcept;

    protected:
        explicit GBufferProgram ( std::string_view vs,
            std::string_view fs,
            std::string_view name,
            size_t pushConstantSize
        ) noexcept;

        ~GBufferProgram () override = default;

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


#endif // PBR_GBUFFER_PROGRAM_HPP
