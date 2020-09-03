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
        Program ( const Program &other ) = delete;
        Program& operator = ( const Program &other ) = delete;

        // Methods must be called between changing input shader parameters: VkImageView, VkSampler, VkBuffer.
        [[maybe_unused]] virtual void BeginSetup () = 0;
        [[maybe_unused]] virtual void EndSetup () = 0;

        // Method commits active material parameters and assigns VkPipeline as active pipeline.
        // Method return true is success. Otherwise method returns false.
        [[maybe_unused]] [[nodiscard]] virtual bool Bind ( android_vulkan::Renderer &renderer ) = 0;

        // Method return true is success. Otherwise method returns false.
        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            const VkExtent2D &viewport
        ) = 0;

        [[maybe_unused]] virtual void Destroy ( android_vulkan::Renderer &renderer ) = 0;
        [[nodiscard]] virtual const std::vector<ProgramResource>& GetResourceInfo () const = 0;

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout () const;

    protected:
        explicit Program ( std::string &&name );
        virtual ~Program () = default;

        [[nodiscard]] virtual const VkPipelineColorBlendStateCreateInfo* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const = 0;

        [[nodiscard]] virtual const VkPipelineDepthStencilStateCreateInfo* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual const VkPipelineInputAssemblyStateCreateInfo* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual bool InitLayout ( VkPipelineLayout &layout,
            android_vulkan::Renderer &renderer
        ) = 0;

        [[nodiscard]] virtual const VkPipelineMultisampleStateCreateInfo* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual const VkPipelineRasterizationStateCreateInfo* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const = 0;

        [[nodiscard]] virtual bool InitShaderInfo ( const VkPipelineShaderStageCreateInfo* &targetInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo,
            android_vulkan::Renderer &renderer
        ) = 0;

        [[nodiscard]] virtual const VkPipelineViewportStateCreateInfo* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D &scissorInfo,
            VkViewport &viewportInfo,
            const VkExtent2D &viewport
        ) const = 0;

        [[nodiscard]] virtual const VkPipelineVertexInputStateCreateInfo* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const = 0;
};

} // namespace pbr


#endif // PBR_PROGRAM_H
