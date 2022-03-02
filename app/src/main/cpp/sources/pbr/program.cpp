#include <pbr/program.h>


namespace pbr {

void Program::Bind ( VkCommandBuffer commandBuffer ) const noexcept
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
}

Program::Program ( std::string_view &&name ) noexcept:
    _fragmentShader ( VK_NULL_HANDLE ),
    _vertexShader ( VK_NULL_HANDLE ),
    _name ( name ),
    _pipeline ( VK_NULL_HANDLE ),
    _pipelineLayout ( VK_NULL_HANDLE )
{
    // NOTHING
}

} // namespace pbr
