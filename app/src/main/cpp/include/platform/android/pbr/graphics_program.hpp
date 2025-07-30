#ifndef PBR_ANDROID_GRAPHICS_PROGRAM_HPP
#define PBR_ANDROID_GRAPHICS_PROGRAM_HPP


#include <pbr/graphics_program.hpp>
#include <renderer.hpp>


namespace pbr::android {

class GraphicsProgram : public pbr::GraphicsProgram
{
    protected:
        VkShaderModule      _fragmentShader = VK_NULL_HANDLE;
        VkShaderModule      _vertexShader = VK_NULL_HANDLE;

    public:
        GraphicsProgram () = delete;

        GraphicsProgram ( GraphicsProgram const & ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram const & ) = delete;

        GraphicsProgram ( GraphicsProgram && ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram && ) = delete;

        // Successor classes MUST call this method.
        void Destroy ( VkDevice device ) noexcept override;

    protected:
        explicit GraphicsProgram ( std::string_view name ) noexcept;
        ~GraphicsProgram () override = default;

        [[nodiscard]] virtual bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            VkPipelineShaderStageCreateInfo const* &targetInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) noexcept = 0;

        [[nodiscard]] virtual VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const noexcept = 0;

        void DestroyShaderModules ( VkDevice device ) noexcept;
};

} // namespace pbr::android


#endif // PBR_ANDROID_GRAPHICS_PROGRAM_HPP
