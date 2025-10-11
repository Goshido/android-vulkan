#include <precompiled_headers.hpp>
#include <platform/android/mandelbrot/mandelbrot_analytic_color.hpp>
#include <vulkan_utils.hpp>


namespace mandelbrot {

namespace {

constexpr char const* FRAGMENT_SHADER = "shaders/mandelbrot_analytic_color.ps.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

MandelbrotAnalyticColor::MandelbrotAnalyticColor () noexcept:
    MandelbrotBase ( FRAGMENT_SHADER )
{
    // NOTHING
}

bool MandelbrotAnalyticColor::CreatePipelineLayout ( VkDevice device ) noexcept
{
    constexpr VkPipelineLayoutCreateInfo pipelineLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 0U,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &_pipelineLayout ),
        "MandelbrotAnalyticColor::CreatePipeline",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _pipelineLayout,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        "MandelbrotAnalyticColor::_pipelineLayout"
    )

    return true;
}

void MandelbrotAnalyticColor::DestroyPipelineLayout ( VkDevice device ) noexcept
{
    if ( _pipelineLayout == VK_NULL_HANDLE )
        return;

    vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
    _pipelineLayout = VK_NULL_HANDLE;
}

bool MandelbrotAnalyticColor::RecordCommandBuffer ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFramebuffer framebuffer
) noexcept
{
    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .pInheritanceInfo = nullptr
    };

    constexpr VkClearValue colorClearValue
    {
        .color
        {
            .float32 { 0.0F, 0.0F, 0.0F, 1.0F }
        }
    };

    VkRenderPassBeginInfo renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = _renderPass,
        .framebuffer = VK_NULL_HANDLE,

        .renderArea
        {
            .offset
            {
                .x = 0,
                .y = 0
            },

            .extent = renderer.GetSurfaceSize ()
        },

        .clearValueCount = 1U,
        .pClearValues = &colorClearValue,
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
        "MandelbrotAnalyticColor::CreateCommandBuffer",
        "Can't begin command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    renderPassBeginInfo.framebuffer = framebuffer;

    vkCmdBeginRenderPass ( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
    vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
    vkCmdEndRenderPass ( commandBuffer );

    return android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "MandelbrotAnalyticColor::CreateCommandBuffer",
        "Can't end command buffer"
    );
}

} // namespace mandelbrot
