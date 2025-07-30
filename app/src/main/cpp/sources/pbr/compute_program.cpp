#include <precompiled_headers.hpp>
#include <pbr/compute_program.hpp>
#include <vulkan_api.hpp>


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
        vkDestroyPipelineLayout ( device, std::exchange ( _pipelineLayout, VK_NULL_HANDLE ), nullptr );

    if ( _pipeline != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyPipeline ( device, std::exchange ( _pipeline, VK_NULL_HANDLE ), nullptr );
    }
}

} // namespace pbr
