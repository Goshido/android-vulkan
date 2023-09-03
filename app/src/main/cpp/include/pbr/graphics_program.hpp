#ifndef PBR_PROGRAM_HPP
#define PBR_PROGRAM_HPP


#include <renderer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static char const* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr static char const* FRAGMENT_SHADER_ENTRY_POINT = "PS";

//----------------------------------------------------------------------------------------------------------------------

class GraphicsProgram
{
    public:
        using SetItem = std::vector<VkDescriptorPoolSize>;
        using DescriptorSetInfo = std::vector<SetItem>;
        using SpecializationData = void const*;

    protected:
        VkShaderModule                              _fragmentShader = VK_NULL_HANDLE;
        VkShaderModule                              _vertexShader = VK_NULL_HANDLE;

        [[maybe_unused]] std::string_view const     _name;
        VkPipeline                                  _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout                            _pipelineLayout = VK_NULL_HANDLE;

    public:
        GraphicsProgram () = delete;

        GraphicsProgram ( GraphicsProgram const & ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram const & ) = delete;

        GraphicsProgram ( GraphicsProgram && ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram && ) = delete;

        // Method return true is success. Otherwise method returns false.
        // The method MUST invoke vkCreateGraphicsPipelines at the end.
        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            SpecializationData specializationData,
            VkExtent2D const &viewport
        ) noexcept = 0;

        virtual void Destroy ( VkDevice device ) noexcept = 0;
        [[nodiscard]] virtual DescriptorSetInfo const &GetResourceInfo () const noexcept = 0;

        // The method assigns VkPipeline as active pipeline.
        void Bind ( VkCommandBuffer commandBuffer ) const noexcept;

    protected:
        explicit GraphicsProgram ( std::string_view name ) noexcept;
        virtual ~GraphicsProgram () = default;

        [[nodiscard]] virtual VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineInputAssemblyStateCreateInfo const* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept = 0;

        [[nodiscard]] virtual VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const noexcept = 0;

        [[nodiscard]] virtual bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            VkPipelineShaderStageCreateInfo const* &targetInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) noexcept = 0;

        virtual void DestroyShaderModules ( VkDevice device ) noexcept = 0;

        [[nodiscard]] virtual VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D &scissorInfo,
            VkViewport &viewportInfo,
            VkExtent2D const &viewport
        ) const noexcept = 0;

        [[nodiscard]] virtual VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const noexcept = 0;
};

} // namespace pbr


#endif // PBR_PROGRAM_HPP
