#include <precompiled_headers.hpp>
#include <pbr/graphics_program_base.hpp>
#include <vulkan_api.hpp>


namespace pbr {

void GraphicsProgramBase::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
}

GraphicsProgramBase::GraphicsProgramBase ( size_t pushConstantSize ) noexcept:
    Program ( pushConstantSize )
{
    // NOTHING
}

} // namespace pbr
