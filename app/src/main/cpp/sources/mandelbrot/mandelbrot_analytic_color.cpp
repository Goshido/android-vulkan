#include <mandelbrot/mandelbrot_analytic_color.h>
#include <vulkan_utils.h>


namespace mandelbrot {

constexpr static char const* FRAGMENT_SHADER = "shaders/mandelbrot-analytic-color-ps.spv";

MandelbrotAnalyticColor::MandelbrotAnalyticColor () noexcept:
    MandelbrotBase ( FRAGMENT_SHADER )
{
    // NOTHING
}

bool MandelbrotAnalyticColor::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !MandelbrotBase::OnSwapchainCreated ( renderer ) )
        return false;

    if ( CreateCommandBuffer ( renderer ) )
        return true;

    OnSwapchainDestroyed ( renderer.GetDevice () );
    return false;
}

void MandelbrotAnalyticColor::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    DestroyCommandBuffer ( device );
    MandelbrotBase::OnSwapchainDestroyed ( device );
}

bool MandelbrotAnalyticColor::CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept
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
        vkCreatePipelineLayout ( renderer.GetDevice (), &pipelineLayoutInfo, nullptr, &_pipelineLayout ),
        "MandelbrotAnalyticColor::CreatePipeline",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "MandelbrotAnalyticColor::_pipelineLayout" )
    return true;
}

void MandelbrotAnalyticColor::DestroyPipelineLayout ( VkDevice device ) noexcept
{
    if ( _pipelineLayout == VK_NULL_HANDLE )
        return;

    vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
    _pipelineLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE_LAYOUT ( "MandelbrotAnalyticColor::_pipelineLayout" )
}

bool MandelbrotAnalyticColor::CreateCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    size_t const framebufferCount = _framebuffers.size ();
    _commandBuffer.resize ( framebufferCount );

    VkCommandBufferAllocateInfo const commandBufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( framebufferCount )
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &commandBufferInfo, _commandBuffer.data () ),
        "MandelbrotAnalyticColor::CreateCommandBuffer",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        DestroyCommandBuffer ( device );
        return false;
    }

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = nullptr
    };

    constexpr VkClearValue colorClearValue
    {
        .color
        {
            .float32 { 0.0F, 0.0F, 0.0F, 1.0F }
        }
    };

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

        result = android_vulkan::Renderer::CheckVkResult (
            vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
            "MandelbrotAnalyticColor::CreateCommandBuffer",
            "Can't begin command buffer"
        );

        if ( !result )
        {
            DestroyCommandBuffer ( device );
            return false;
        }

        renderPassBeginInfo.framebuffer = _framebuffers[ i ];

        vkCmdBeginRenderPass ( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
        vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
        vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
        vkCmdEndRenderPass ( commandBuffer );

        result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
            "MandelbrotAnalyticColor::CreateCommandBuffer",
            "Can't end command buffer"
        );
    }

    if ( result )
        return true;

    DestroyCommandBuffer ( device );
    return false;
}

void MandelbrotAnalyticColor::DestroyCommandBuffer ( VkDevice device ) noexcept
{
    if ( _commandBuffer.empty () )
        return;

    vkFreeCommandBuffers ( device,
        _commandPool,
        static_cast<uint32_t> ( _commandBuffer.size () ),
        _commandBuffer.data ()
    );

    _commandBuffer.clear ();
    _commandBuffer.shrink_to_fit ();
}

} // namespace mandelbrot
