#ifndef PBR_PROGRAM_H
#define PBR_PROGRAM_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE

#include <renderer.h>
#include "program_resource.h"


namespace pbr {

constexpr static const char* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr static const char* FRAGMENT_SHADER_ENTRY_POINT = "PS";

//----------------------------------------------------------------------------------------------------------------------

class Program
{
    protected:
        VkShaderModule          _fragmentShader;
        VkShaderModule          _vertexShader;

        const std::string       _name;
        VkPipeline              _pipeline;
        VkPipelineLayout        _pipelineLayout;

    public:
        Program () = delete;
        Program ( Program const &other ) = delete;
        Program& operator = ( Program const &other ) = delete;
        Program ( Program &&other ) = delete;
        Program& operator = ( Program &&other ) = delete;

        // Method return true is success. Otherwise method returns false.
        // The method MUST invoke vkCreateGraphicsPipelines at the end.
        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            VkExtent2D const &viewport
        ) = 0;

        virtual void Destroy ( android_vulkan::Renderer &renderer ) = 0;
        [[nodiscard]] virtual std::vector<DescriptorSetInfo> const& GetResourceInfo () const = 0;

        // The method assigns VkPipeline as active pipeline.
        void Bind ( VkCommandBuffer commandBuffer ) const;

    protected:
        explicit Program ( std::string &&name ) noexcept;
        virtual ~Program () = default;

        [[nodiscard]] virtual VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const = 0;

        [[nodiscard]] virtual VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual VkPipelineInputAssemblyStateCreateInfo const* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual bool InitLayout ( VkPipelineLayout &layout,
            android_vulkan::Renderer &renderer
        ) = 0;

        [[nodiscard]] virtual VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual bool InitShaderInfo ( VkPipelineShaderStageCreateInfo const* &targetInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo,
            android_vulkan::Renderer &renderer
        ) = 0;

        [[nodiscard]] virtual VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D &scissorInfo,
            VkViewport &viewportInfo,
            VkExtent2D const &viewport
        ) const = 0;

        [[nodiscard]] virtual VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const = 0;
};

} // namespace pbr


#endif // PBR_PROGRAM_H
