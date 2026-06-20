#include <precompiled_headers.hpp>
#include <platform/windows/pbr/compute_program.hpp>


namespace pbr {

VkPipelineLayout ComputeProgram::GetPipelineLayout () const noexcept
{
    return _pipelineLayout;
}

ComputeProgram::ComputeProgram ( size_t pushConstantSize ) noexcept:
    ComputeProgramBase ( pushConstantSize )
{
    // NOTHING
}

} // namespace pbr
