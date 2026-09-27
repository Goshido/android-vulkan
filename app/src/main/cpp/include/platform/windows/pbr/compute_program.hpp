#ifndef PBR_COMPUTE_PROGRAM_HPP
#define PBR_COMPUTE_PROGRAM_HPP


#include <pbr/compute_program_base.hpp>


namespace pbr {

class ComputeProgram : public ComputeProgramBase
{
    public:
        ComputeProgram () = delete;

        ComputeProgram ( ComputeProgram const & ) = delete;
        ComputeProgram &operator = ( ComputeProgram const & ) = delete;

        ComputeProgram ( ComputeProgram && ) = delete;
        ComputeProgram &operator = ( ComputeProgram && ) = delete;

        void SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept;

    protected:
        explicit ComputeProgram ( size_t pushConstantSize ) noexcept;
        ~ComputeProgram () override = default;

        [[nodiscard]] virtual bool Init ( VkDevice device, SpecializationData specializationData ) noexcept = 0;

        [[nodiscard]] virtual VkPipelineShaderStageCreateInfo InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo
        ) noexcept = 0;
};

} // namespace pbr


#endif // PBR_COMPUTE_PROGRAM_HPP
