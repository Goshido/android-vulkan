#include <pbr/program.h>


namespace pbr {

ProgramResource::ProgramResource ( VkDescriptorType type, uint32_t count ):
    _type ( type ),
    _count ( count )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

void Program::Bind ( VkCommandBuffer commandBuffer ) const
{
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
}

VkDescriptorSetLayout Program::GetDescriptorSetLayout () const
{
    return _descriptorSetLayout;
}

Program::Program ( std::string &&name ):
    _fragmentShader ( VK_NULL_HANDLE ),
    _vertexShader ( VK_NULL_HANDLE ),
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _name ( std::move ( name ) ),
    _pipeline ( VK_NULL_HANDLE ),
    _pipelineLayout ( VK_NULL_HANDLE ),
    _state ( eProgramState::Unknown )
{
    // NOTHING
}

} // namespace pbr
