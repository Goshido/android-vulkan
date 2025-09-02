// FUCK - remove namespace
#ifndef PBR_ANDROID_COMPUTE_PROGRAM_HPP
#define PBR_ANDROID_COMPUTE_PROGRAM_HPP


#include <pbr/compute_program.hpp>
#include <renderer.hpp>


namespace pbr::android {

class ComputeProgram : public pbr::ComputeProgram
{
    protected:
        VkShaderModule      _computeShader = VK_NULL_HANDLE;

    public:
        ComputeProgram () = delete;

        ComputeProgram ( ComputeProgram const & ) = delete;
        ComputeProgram &operator = ( ComputeProgram const & ) = delete;

        ComputeProgram ( ComputeProgram && ) = delete;
        ComputeProgram &operator = ( ComputeProgram && ) = delete;

    protected:
        explicit ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept;
        ~ComputeProgram () override = default;

        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData
        ) noexcept = 0;

        // Successor classes MUST call this method.
        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] virtual bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept = 0;

        void DestroyShaderModule ( VkDevice device ) noexcept;
};

} // namespace pbr::android


#endif // PBR_ANDROID_COMPUTE_PROGRAM_HPP
