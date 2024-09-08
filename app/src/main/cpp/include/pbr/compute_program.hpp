#ifndef PBR_COMPUTE_PROGRAM_HPP
#define PBR_COMPUTE_PROGRAM_HPP


#include <renderer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static char const* COMPUTE_SHADER_ENTRY_POINT = "CS";

//----------------------------------------------------------------------------------------------------------------------

class ComputeProgram
{
    public:
        using SpecializationData = void const*;

    protected:
        VkShaderModule                              _computeShader = VK_NULL_HANDLE;

        [[maybe_unused]] std::string_view const     _name;
        VkPipeline                                  _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout                            _pipelineLayout = VK_NULL_HANDLE;
        uint32_t                                    _pushConstantSize;

    public:
        ComputeProgram () = delete;

        ComputeProgram ( ComputeProgram const & ) = delete;
        ComputeProgram &operator = ( ComputeProgram const & ) = delete;

        ComputeProgram ( ComputeProgram && ) = delete;
        ComputeProgram &operator = ( ComputeProgram && ) = delete;


        // The method assigns VkPipeline as active pipeline.
        void Bind ( VkCommandBuffer commandBuffer ) const noexcept;

        void SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept;

    protected:
        explicit ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept;
        virtual ~ComputeProgram () = default;

        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData
        ) noexcept = 0;

        // Successor classes MUST call this method.
        virtual void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] virtual bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept = 0;

        [[nodiscard]] virtual bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept = 0;

        void DestroyShaderModule ( VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_COMPUTE_PROGRAM_HPP
