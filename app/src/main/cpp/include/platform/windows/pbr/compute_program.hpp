#ifndef PBR_WINDOWS_COMPUTE_PROGRAM_HPP
#define PBR_WINDOWS_COMPUTE_PROGRAM_HPP


#include <pbr/compute_program.hpp>
#include <renderer.hpp>


namespace pbr::windows {

class ComputeProgram : public pbr::ComputeProgram
{
    public:
        ComputeProgram () = delete;

        ComputeProgram ( ComputeProgram const & ) = delete;
        ComputeProgram &operator = ( ComputeProgram const & ) = delete;

        ComputeProgram ( ComputeProgram && ) = delete;
        ComputeProgram &operator = ( ComputeProgram && ) = delete;

    protected:
        explicit ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept;
        ~ComputeProgram () override = default;

        [[nodiscard]] virtual bool Init ( VkDevice device, SpecializationData specializationData ) noexcept = 0;

        [[nodiscard]] virtual bool InitShaderInfo ( VkDevice device,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept = 0;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_COMPUTE_PROGRAM_HPP
