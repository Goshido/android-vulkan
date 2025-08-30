#include <precompiled_headers.hpp>
#include <platform/windows/pbr/graphics_program.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::windows {

GraphicsProgram::GraphicsProgram ( std::string_view name, size_t pushConstantSize ) noexcept:
    pbr::GraphicsProgram ( name ),
    _pushConstantSize ( static_cast<uint32_t> ( pushConstantSize ) )
{
    // NOTHING
}

void GraphicsProgram::SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept
{
    constexpr VkPipelineStageFlags stages =
        AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT );

    vkCmdPushConstants ( commandBuffer, _pipelineLayout, stages, 0U, _pushConstantSize, constants );
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
