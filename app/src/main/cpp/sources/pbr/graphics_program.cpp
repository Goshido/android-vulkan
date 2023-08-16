#include <pbr/graphics_program.hpp>


namespace pbr {

void GraphicsProgram::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
}

GraphicsProgram::GraphicsProgram ( std::string_view &&name ) noexcept:
    _name ( name )
{
    // NOTHING
}

} // namespace pbr
