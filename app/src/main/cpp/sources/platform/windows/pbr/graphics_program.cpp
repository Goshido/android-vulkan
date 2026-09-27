#include <precompiled_headers.hpp>
#include <platform/windows/pbr/graphics_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <vulkan_api.hpp>


namespace pbr {

void GraphicsProgram::Destroy ( VkDevice device ) noexcept
{
    GraphicsProgramBase::Destroy ( device );
}

GraphicsProgram::GraphicsProgram ( size_t pushConstantSize ) noexcept:
    GraphicsProgramBase ( pushConstantSize )
{
    // NOTHING
}

void GraphicsProgram::SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept
{
    vkCmdPushConstants ( commandBuffer,
        UniversalPipelineLayout::GetPipelineLayout (),
        UniversalPipelineLayout::GetStages (),
        0U,
        _pushConstantSize,
        constants
    );
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

} // namespace pbr
