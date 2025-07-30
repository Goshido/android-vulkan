#include <precompiled_headers.hpp>
#include <platform/windows/pbr/compute_program.hpp>


namespace pbr::windows {

ComputeProgram::ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept:
    pbr::ComputeProgram ( name, pushConstantSize )
{
    // NOTHING
}

} // namespace pbr::windows
