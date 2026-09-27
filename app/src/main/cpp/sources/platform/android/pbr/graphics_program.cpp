#include <precompiled_headers.hpp>
#include <platform/android/pbr/graphics_program.hpp>


namespace pbr {

void GraphicsProgram::Destroy ( VkDevice device ) noexcept
{
    GraphicsProgramBase::Destroy ( device );

    if ( _pipelineLayout != VK_NULL_HANDLE ) [[likely]]
        vkDestroyPipelineLayout ( device, std::exchange ( _pipelineLayout, VK_NULL_HANDLE ), nullptr );

    DestroyShaderModules ( device );
}

GraphicsProgram::GraphicsProgram ( size_t pushConstantSize ) noexcept:
    GraphicsProgramBase ( pushConstantSize )
{
    // NOTHING
}

void GraphicsProgram::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _fragmentShader != VK_NULL_HANDLE )
        vkDestroyShaderModule ( device, std::exchange ( _fragmentShader, VK_NULL_HANDLE ), nullptr );

    if ( _vertexShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, std::exchange ( _vertexShader, VK_NULL_HANDLE ), nullptr );
    }
}

} // namespace pbr
