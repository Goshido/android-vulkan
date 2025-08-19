#include <precompiled_headers.hpp>
#include <platform/windows/pbr/graphics_program.hpp>


// FUCK - remove namespace
namespace pbr::windows {

GraphicsProgram::GraphicsProgram ( std::string_view name ) noexcept:
    pbr::GraphicsProgram ( name )
{
    // NOTHING
}

VkPipelineVertexInputStateCreateInfo const* GraphicsProgram::InitVertexInputInfo () noexcept
{
    constexpr static VkPipelineVertexInputStateCreateInfo info
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .vertexBindingDescriptionCount = 0U,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0U,
        .pVertexAttributeDescriptions = nullptr
    };

    return &info;
}

} // namespace pbr::windows
