#include <precompiled_headers.hpp>
#include <pbr/compute_program_base.hpp>
#include <vulkan_api.hpp>


namespace pbr {

void ComputeProgramBase::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline );
}

ComputeProgramBase::ComputeProgramBase ( size_t pushConstantSize ) noexcept:
    Program ( pushConstantSize )
{
    // NOTHING
}

} // namespace pbr
