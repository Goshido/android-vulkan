#include <mandelbrot/mandelbrot_analytic_color.h>
#include <vulkan_utils.h>


namespace mandelbrot {

constexpr static const char* FRAGMENT_SHADER = "shaders/mandelbrot-analytic-color-ps.spv";

MandelbrotAnalyticColor::MandelbrotAnalyticColor ():
    MandelbrotBase ( FRAGMENT_SHADER )
{
    // NOTHING
}

bool MandelbrotAnalyticColor::OnInit ( android_vulkan::Renderer &renderer )
{
    if ( !MandelbrotBase::OnInit ( renderer ) )
        return false;

    if ( CreateCommandBuffer ( renderer ) )
        return true;

    OnDestroy ( renderer );
    return false;
}

bool MandelbrotAnalyticColor::OnDestroy ( android_vulkan::Renderer &renderer )
{
    const bool result = renderer.CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "MandelbrotAnalyticColor::OnDestroy",
        "Can't wait queue idle"
    );

    if ( !result )
        return false;

    DestroyCommandBuffer ( renderer );
    return MandelbrotBase::OnDestroy ( renderer );
}

bool MandelbrotAnalyticColor::CreatePipelineLayout ( android_vulkan::Renderer &renderer )
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.flags = 0U;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0U;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    pipelineLayoutInfo.setLayoutCount = 0U;
    pipelineLayoutInfo.pSetLayouts = nullptr;

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &_pipelineLayout ),
        "MandelbrotAnalyticColor::CreatePipeline",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "MandelbrotAnalyticColor::_pipelineLayout" )
    return true;
}

void MandelbrotAnalyticColor::DestroyPipelineLayout ( android_vulkan::Renderer &renderer )
{
    if ( _pipelineLayout == VK_NULL_HANDLE )
        return;

    vkDestroyPipelineLayout ( renderer.GetDevice (), _pipelineLayout, nullptr );
    _pipelineLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE_LAYOUT ( "MandelbrotAnalyticColor::_pipelineLayout" )
}

bool MandelbrotAnalyticColor::CreateCommandBuffer ( android_vulkan::Renderer &renderer )
{
    const size_t framebufferCount = _framebuffers.size ();
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
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    VkClearValue colorClearValue;
    colorClearValue.color.float32[ 0U ] = 0.0F;
    colorClearValue.color.float32[ 1U ] = 0.0F;
    colorClearValue.color.float32[ 2U ] = 0.0F;
    colorClearValue.color.float32[ 3U ] = 1.0F;

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = _renderPass;

    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();
    renderPassBeginInfo.clearValueCount = 1U;
    renderPassBeginInfo.pClearValues = &colorClearValue;

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        VkCommandBuffer commandBuffer = _commandBuffer[ i ];

        result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
            "MandelbrotAnalyticColor::CreateCommandBuffer",
            "Can't begin command buffer"
        );

        if ( !result )
        {
            DestroyCommandBuffer ( renderer );
            return false;
        }

        renderPassBeginInfo.framebuffer = _framebuffers[ i ];

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
