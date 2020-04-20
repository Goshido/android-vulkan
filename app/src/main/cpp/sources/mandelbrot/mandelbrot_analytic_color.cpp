#include <mandelbrot/mandelbrot_analytic_color.h>


namespace mandelbrot {

constexpr static const char* FRAGMENT_SHADER = "shaders/mandelbrot-analytic-color-ps.spv";

MandelbrotAnalyticColor::MandelbrotAnalyticColor ():
    MandelbrotBase ( FRAGMENT_SHADER )
{
    // NOTHING
}

bool MandelbrotAnalyticColor::OnDestroy ( android_vulkan::Renderer &renderer )
{
    DestroyCommandBuffer ( renderer );
    return MandelbrotBase::OnDestroy ( renderer );
}

bool MandelbrotAnalyticColor::CreateCommandBuffer ( android_vulkan::Renderer &renderer )
{
    const size_t framebufferCount = renderer.GetPresentFramebufferCount ();
    _commandBuffer.resize ( framebufferCount );

    VkCommandBufferAllocateInfo commandBufferInfo;
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.pNext = nullptr;
    commandBufferInfo.commandBufferCount = static_cast<uint32_t> ( framebufferCount );
    commandBufferInfo.commandPool = _commandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    bool result = renderer.CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &commandBufferInfo, _commandBuffer.data () ),
        "MandelbrotAnalyticColor::CreateCommandBuffer",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        DestroyCommandBuffer ( renderer );
        return false;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = 0U;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    VkClearValue clearValues[ 2U ];
    VkClearValue& colorClearValue = clearValues[ 0U ];
    colorClearValue.color.float32[ 0U ] = 0.0F;
    colorClearValue.color.float32[ 1U ] = 0.0F;
    colorClearValue.color.float32[ 2U ] = 0.0F;
    colorClearValue.color.float32[ 3U ] = 1.0F;

    VkClearValue& depthStencilClearValue = clearValues[ 1U ];
    depthStencilClearValue.depthStencil.depth = 1.0F;
    depthStencilClearValue.depthStencil.stencil = 0U;

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = _renderPass;

    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();
    renderPassBeginInfo.clearValueCount = 2U;
    renderPassBeginInfo.pClearValues = clearValues;

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        const VkCommandBuffer commandBuffer = _commandBuffer[ i ];

        result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
            "MandelbrotAnalyticColor::CreateCommandBuffer", "Can't begin command buffer"
        );

        if ( !result )
        {
            DestroyCommandBuffer ( renderer );
            return false;
        }

        renderPassBeginInfo.framebuffer = renderer.GetPresentFramebuffer ( static_cast<uint32_t> ( i ) );

        vkCmdBeginRenderPass ( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
        vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
        vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
        vkCmdEndRenderPass ( commandBuffer );

        result = renderer.CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
            "MandelbrotAnalyticColor::CreateCommandBuffer",
            "Can't end command buffer"
        );
    }

    if ( result )
        return true;

    DestroyCommandBuffer ( renderer );
    return false;
}

void MandelbrotAnalyticColor::DestroyCommandBuffer ( android_vulkan::Renderer& /*renderer*/ )
{
    _commandBuffer.clear ();
}

} // namespace mandelbrot
