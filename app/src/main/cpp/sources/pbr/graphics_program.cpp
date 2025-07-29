#include <precompiled_headers.hpp>
#include <pbr/graphics_program.hpp>
#include <vulkan_api.hpp>


namespace pbr {

void GraphicsProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipeline != VK_NULL_HANDLE ) [[likely]]
        vkDestroyPipeline ( device, std::exchange ( _pipeline, VK_NULL_HANDLE ), nullptr );

    if ( _pipelineLayout != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyPipelineLayout ( device, std::exchange ( _pipelineLayout, VK_NULL_HANDLE ), nullptr );
    }
}

void GraphicsProgram::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
}

GraphicsProgram::GraphicsProgram ( std::string_view name ) noexcept:
    _name ( name )
{
    // NOTHING
}

} // namespace pbr
