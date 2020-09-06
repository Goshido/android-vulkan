#ifndef PBR_PROGRAM_H
#define PBR_PROGRAM_H


#include <renderer.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static const char* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr static const char* FRAGMENT_SHADER_ENTRY_POINT = "PS";

enum class eProgramState : uint8_t
{
    Bind,
    Initializing,
    Ready,
    Setup,
    Unknown
};

struct ProgramResource final
{
    VkDescriptorType    _type;
    uint32_t            _count;

    ProgramResource () = delete;
    explicit ProgramResource ( VkDescriptorType type, uint32_t count );

    ProgramResource ( const ProgramResource &other ) = delete;
    ProgramResource& operator = ( const ProgramResource &other ) = delete;

    ProgramResource ( ProgramResource &&other ) = default;
    ProgramResource& operator = ( ProgramResource &&other ) = default;
};

//----------------------------------------------------------------------------------------------------------------------

class Program
{
    protected:
        VkShaderModule                  _fragmentShader;
        VkShaderModule                  _vertexShader;

        VkDescriptorSetLayout           _descriptorSetLayout;
        const std::string               _name;
        VkPipeline                      _pipeline;
        VkPipelineLayout                _pipelineLayout;

        eProgramState                   _state;

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

        [[maybe_unused]] virtual void Destroy ( android_vulkan::Renderer &renderer ) = 0;
        [[nodiscard]] virtual std::vector<ProgramResource> const& GetResourceInfo () const = 0;

        // The method assigns VkPipeline as active pipeline.
        void Bind ( VkCommandBuffer commandBuffer ) const;

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout () const;

    protected:
        explicit Program ( std::string &&name );
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
