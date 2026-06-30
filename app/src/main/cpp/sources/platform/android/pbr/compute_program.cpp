#include <precompiled_headers.hpp>
#include <platform/android/pbr/compute_program.hpp>


namespace pbr {

void ComputeProgram::SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept
{
    vkCmdPushConstants ( commandBuffer,
        _pipelineLayout,
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

void ComputeProgram::Destroy ( VkDevice device ) noexcept
{
    ComputeProgramBase::Destroy ( device );

    if ( _pipelineLayout != VK_NULL_HANDLE ) [[likely]]
        vkDestroyPipelineLayout ( device, std::exchange ( _pipelineLayout, VK_NULL_HANDLE ), nullptr );

    DestroyShaderModule ( device );
}

void ComputeProgram::DestroyShaderModule ( VkDevice device ) noexcept
{
    if ( _computeShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, std::exchange ( _computeShader, VK_NULL_HANDLE ), nullptr );
    }
}

} // namespace pbr
