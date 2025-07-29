#include <precompiled_headers.hpp>
#include <platform/android/pbr/graphics_program.hpp>


namespace pbr::android {

void GraphicsProgram::Destroy ( VkDevice device ) noexcept
{
    pbr::GraphicsProgram::Destroy ( device );
    DestroyShaderModules ( device );
}

GraphicsProgram::GraphicsProgram ( std::string_view name ) noexcept:
    pbr::GraphicsProgram ( name )
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

} // namespace pbr::android
