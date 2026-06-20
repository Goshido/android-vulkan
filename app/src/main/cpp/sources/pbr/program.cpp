#include <precompiled_headers.hpp>
#include <pbr/program.hpp>
#include <vulkan_api.hpp>


namespace pbr {

void Program::Destroy ( VkDevice device ) noexcept
{
    if ( _pipelineLayout != VK_NULL_HANDLE ) [[likely]]
        vkDestroyPipelineLayout ( device, std::exchange ( _pipelineLayout, VK_NULL_HANDLE ), nullptr );

    if ( _pipeline != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyPipeline ( device, std::exchange ( _pipeline, VK_NULL_HANDLE ), nullptr );
    }
}

Program::Program ( size_t pushConstantSize ) noexcept:
    _pushConstantSize ( static_cast<uint32_t> ( pushConstantSize ) )
{
    // NOTHING
}

} // namespace pbr
