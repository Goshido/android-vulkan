#ifndef PBR_GRAPHICS_PROGRAM_BASE_HPP
#define PBR_GRAPHICS_PROGRAM_BASE_HPP


#include "program.hpp"


namespace pbr {

constexpr static char const* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr static char const* FRAGMENT_SHADER_ENTRY_POINT = "PS";

//----------------------------------------------------------------------------------------------------------------------

class GraphicsProgramBase : public Program
{
    public:
        GraphicsProgramBase () = delete;

        GraphicsProgramBase ( GraphicsProgramBase const & ) = delete;
        GraphicsProgramBase &operator = ( GraphicsProgramBase const & ) = delete;

        GraphicsProgramBase ( GraphicsProgramBase && ) = delete;
        GraphicsProgramBase &operator = ( GraphicsProgramBase && ) = delete;

        // The method assigns VkPipeline as active pipeline.
        void Bind ( VkCommandBuffer commandBuffer ) const noexcept;

    protected:
        explicit GraphicsProgramBase ( size_t pushConstantSize ) noexcept;
        virtual ~GraphicsProgramBase () = default;

        [[nodiscard]] virtual VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineDynamicStateCreateInfo const* InitDynamicStateInfo (
            VkPipelineDynamicStateCreateInfo* info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineInputAssemblyStateCreateInfo const* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D* scissorInfo,
            VkViewport* viewportInfo,
            VkExtent2D const* viewport
        ) const noexcept = 0;
};

} // namespace pbr


#endif // PBR_GRAPHICS_PROGRAM_BASE_HPP
