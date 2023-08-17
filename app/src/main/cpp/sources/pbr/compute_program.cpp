#include <pbr/compute_program.hpp>


namespace pbr {

void ComputeProgram::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline );
}

ComputeProgram::ComputeProgram ( std::string_view name ) noexcept:
    _name ( name )
{
    // NOTHING
}

} // namespace pbr
