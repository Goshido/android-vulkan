#include <pbr/program.h>


namespace pbr {

void Program::Bind ( VkCommandBuffer commandBuffer ) const
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
}

std::vector<VkDescriptorSetLayout> const& Program::GetDescriptorSetLayouts () const
{
    return _descriptorSetLayouts;
}

Program::Program ( std::string &&name, size_t descriptorSetCount ):
    _fragmentShader ( VK_NULL_HANDLE ),
    _vertexShader ( VK_NULL_HANDLE ),
    _descriptorSetLayouts ( descriptorSetCount, VK_NULL_HANDLE ),
    _name ( std::move ( name ) ),
    _pipeline ( VK_NULL_HANDLE ),
    _pipelineLayout ( VK_NULL_HANDLE ),
    _state ( eProgramState::Unknown )
{
    // NOTHING
}

} // namespace pbr
