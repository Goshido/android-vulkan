#include <precompiled_headers.hpp>
#include <platform/windows/pbr/compute_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <vulkan_api.hpp>


namespace pbr {

void ComputeProgram::SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept
{
    vkCmdPushConstants ( commandBuffer,
        UniversalPipelineLayout::GetPipelineLayout (),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0U,
        _pushConstantSize,
        constants
    );
}

ComputeProgram::ComputeProgram ( size_t pushConstantSize ) noexcept:
    ComputeProgramBase ( pushConstantSize )
{
    // NOTHING
}

} // namespace pbr
