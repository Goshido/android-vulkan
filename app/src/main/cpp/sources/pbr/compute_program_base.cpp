#include <precompiled_headers.hpp>
#include <pbr/compute_program_base.hpp>
#include <vulkan_api.hpp>


namespace pbr {

void ComputeProgramBase::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline );
}

void ComputeProgramBase::SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept
{
    vkCmdPushConstants ( commandBuffer,
        _pipelineLayout,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0U,
        _pushConstantSize,
        constants
    );
}

ComputeProgramBase::ComputeProgramBase ( size_t pushConstantSize ) noexcept:
    Program ( pushConstantSize )
{
    // NOTHING
}

} // namespace pbr
