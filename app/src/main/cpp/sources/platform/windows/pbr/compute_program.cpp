#include <precompiled_headers.hpp>
#include <platform/windows/pbr/compute_program.hpp>


namespace pbr {

VkPipelineLayout ComputeProgram::GetPipelineLayout () const noexcept
{
    return _pipelineLayout;
}

ComputeProgram::ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept:
    ComputeProgramBase ( name, pushConstantSize )
{
    // NOTHING
}

} // namespace pbr
