#include <pbr/compute_program.hpp>


namespace pbr {

void ComputeProgram::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline );
}

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

ComputeProgram::ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept:
    _name ( name ),
    _pushConstantSize ( static_cast<uint32_t> ( pushConstantSize ) )
{
    // NOTHING
}

void ComputeProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipelineLayout != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
    }

    if ( _pipeline != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
    }

    DestroyShaderModule ( device );
}

void ComputeProgram::DestroyShaderModule ( VkDevice device ) noexcept
{
    if ( _computeShader == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyShaderModule ( device, _computeShader, nullptr );
    _computeShader = VK_NULL_HANDLE;
}

} // namespace pbr
