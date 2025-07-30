#include <precompiled_headers.hpp>
#include <platform/android/pbr/compute_program.hpp>


namespace pbr::android {

ComputeProgram::ComputeProgram ( std::string_view name, size_t pushConstantSize ) noexcept:
    pbr::ComputeProgram ( name, pushConstantSize )
{
    // NOTHING
}

void ComputeProgram::Destroy ( VkDevice device ) noexcept
{
    pbr::ComputeProgram::Destroy ( device );
    DestroyShaderModule ( device );
}

void ComputeProgram::DestroyShaderModule ( VkDevice device ) noexcept
{
    if ( _computeShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, std::exchange ( _computeShader, VK_NULL_HANDLE ), nullptr );
    }
}

} // namespace pbr::android
