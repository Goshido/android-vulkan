#include <precompiled_headers.hpp>
#include <platform/windows/pbr/compute_program.hpp>


namespace pbr::windows {

VkPipelineLayout ComputeProgram::GetPipelineLayout () const noexcept
{
    return _pipelineLayout;
}

ComputeProgram::ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept:
    pbr::ComputeProgram ( name, pushConstantSize )
{
    // NOTHING
}

} // namespace pbr::windows
