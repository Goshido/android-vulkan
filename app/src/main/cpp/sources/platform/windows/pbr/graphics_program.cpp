#include <precompiled_headers.hpp>
#include <platform/windows/pbr/graphics_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

GraphicsProgram::GraphicsProgram ( size_t pushConstantSize ) noexcept:
    GraphicsProgramBase ( pushConstantSize )
{
    // NOTHING
}

void GraphicsProgram::Destroy ( VkDevice device ) noexcept
{
    GraphicsProgramBase::Destroy ( device );
}

void GraphicsProgram::SetPushConstants ( VkCommandBuffer commandBuffer, void const* constants ) const noexcept
{
    constexpr VkPipelineStageFlags stages =
        AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT );

    vkCmdPushConstants ( commandBuffer,
        UniversalPipelineLayout::GetPipelineLayout (),
        stages,
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
