#include <rainbow/rainbow.h>

GX_DISABLE_COMMON_WARNINGS

#include <cmath>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace rainbow {

Rainbow::Rainbow ():
    _commandBuffer ( VK_NULL_HANDLE ),
    _commandPool ( VK_NULL_HANDLE ),
    _presentationFence ( VK_NULL_HANDLE ),
    _presentationSemaphore ( VK_NULL_HANDLE ),
    _renderPass ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool Rainbow::IsReady ()
{
    return _presentationSemaphore != VK_NULL_HANDLE;
}

bool Rainbow::OnInit ( android_vulkan::Renderer &renderer )
{
    if ( !CreateRenderPass ( renderer ) )
        return false;

    if ( !CreateCommandBuffer ( renderer ) )
    {
        DestroyRenderPass ( renderer );
        return false;
    }

    if ( CreatePresentationSyncPrimitive ( renderer ) )
        return true;

    DestroyCommandBuffer ( renderer );
    DestroyRenderPass ( renderer );
    return false;
}

bool Rainbow::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime )
{
    uint32_t presentationFramebufferIndex = UINT32_MAX;

    if ( !BeginFrame ( presentationFramebufferIndex, renderer ) )
        return true;

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    bool result = renderer.CheckVkResult ( vkBeginCommandBuffer ( _commandBuffer, &commandBufferBeginInfo ),
        "Rainbow::OnFrame",
        "Can't begin command buffer"
    );

    if ( !result )
        return true;

    constexpr const float PI = 3.14159F;
    constexpr const float TWO_PI = 2.0F * PI;
    constexpr const float CIRCLE_THIRD = TWO_PI / 3.0F;
    constexpr const float CIRCLE_TWO_THIRD = 2.0F * CIRCLE_THIRD;
    constexpr const float RECOLOR_SPEED = 0.777F;

    static float colorFactor = 0.0F;
    colorFactor += RECOLOR_SPEED * static_cast<float> ( deltaTime );

    auto colorSolver = [ & ] ( float offset ) -> float {
        return std::sin ( colorFactor + offset ) * 0.5F + 0.5F;
    };

    VkClearValue clearValues[ 2U ];
    VkClearValue& colorClearValue = clearValues[ 0U ];
    colorClearValue.color.float32[ 0U ] = colorSolver ( 0.0F );
    colorClearValue.color.float32[ 1U ] = colorSolver ( CIRCLE_THIRD );
    colorClearValue.color.float32[ 2U ] = colorSolver ( CIRCLE_TWO_THIRD );
    colorClearValue.color.float32[ 3U ] = 1.0F;

    VkClearValue& depthStencilClearValue = clearValues[ 1U ];
    depthStencilClearValue.depthStencil.depth = 1.0F;
    depthStencilClearValue.depthStencil.stencil = 0U;

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = renderer.GetPresentFramebuffer ( presentationFramebufferIndex );
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();
    renderPassBeginInfo.clearValueCount = 2U;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass ( _commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdEndRenderPass ( _commandBuffer );

    result = renderer.CheckVkResult ( vkEndCommandBuffer ( _commandBuffer ),
        "Rainbow::OnFrame",
        "Can't end command buffer"
    );

    if ( !result )
        return true;

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &_commandBuffer;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores = &_presentationSemaphore;

    result = renderer.CheckVkResult ( vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, _presentationFence ),
        "Rainbow::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return true;

    return EndFrame ( presentationFramebufferIndex, renderer );
}

bool Rainbow::OnDestroy ( android_vulkan::Renderer &renderer )
{
    DestroyPresentationSyncPrimitive ( renderer );
    DestroyCommandBuffer ( renderer );
    DestroyRenderPass ( renderer );

    return true;
}

bool Rainbow::BeginFrame ( uint32_t &presentationFramebufferIndex, android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkAcquireNextImageKHR ( device,
            renderer.GetSwapchain (),
            UINT64_MAX,
            VK_NULL_HANDLE,
            _presentationFence,
            &presentationFramebufferIndex
        ),

        "Rainbow::BeginFrame",
        "Can't acquire next image"
    );

    if ( !result )
        return false;

    result = renderer.CheckVkResult ( vkWaitForFences ( device, 1U, &_presentationFence, VK_TRUE, UINT64_MAX ),
        "Rainbow::BeginFrame",
        "Waiting fence has been failed"
    );

    if ( !result )
        return false;

    result = renderer.CheckVkResult ( vkResetFences ( device, 1U, &_presentationFence ),
        "Rainbow::BeginFrame",
        "Resetting fence has been failed"
    );

    if ( !result )
        return false;

    return renderer.CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "Rainbow::BeginFrame",
        "Waiting queue idle has been failed"
    );
}

bool Rainbow::EndFrame ( uint32_t presentationFramebufferIndex, android_vulkan::Renderer &renderer )
{
    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    VkPresentInfoKHR presentInfoKHR;
    presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfoKHR.pNext = nullptr;
    presentInfoKHR.waitSemaphoreCount = 1U;
    presentInfoKHR.pWaitSemaphores = &_presentationSemaphore;
    presentInfoKHR.swapchainCount = 1U;
    presentInfoKHR.pSwapchains = &renderer.GetSwapchain ();
    presentInfoKHR.pImageIndices = &presentationFramebufferIndex;
    presentInfoKHR.pResults = &presentResult;

    // Note vkQueuePresentKHR may return VK_SUBOPTIMAL_KHR right before application is minimized.
    const VkResult mainResult = vkQueuePresentKHR ( renderer.GetQueue (), &presentInfoKHR );

    if ( mainResult == VK_SUBOPTIMAL_KHR )
        return true;

    if ( !renderer.CheckVkResult ( mainResult, "Rainbow::EndFrame", "Can't present frame" ) )
        return false;

    return renderer.CheckVkResult ( presentResult, "Rainbow::EndFrame", "Present queue has been failed" );
}

bool Rainbow::CreateCommandBuffer ( android_vulkan::Renderer &renderer )
{
    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.queueFamilyIndex = renderer.GetQueueFamilyIndex ();

    commandPoolCreateInfo.flags =
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateCommandPool ( device, &commandPoolCreateInfo, nullptr, &_commandPool ),
        "Rainbow::CreateCommandBuffer",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "Rainbow::_commandPool" )

    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandBufferCount = 1U;
    commandBufferAllocateInfo.commandPool = _commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    result = renderer.CheckVkResult ( vkAllocateCommandBuffers ( device, &commandBufferAllocateInfo, &_commandBuffer ),
        "Rainbow::CreateCommandBuffer",
        "Can't allocate command buffer"
    );

    if ( result )
        return true;

    DestroyCommandBuffer ( renderer );
    return false;
}

void Rainbow::DestroyCommandBuffer ( android_vulkan::Renderer &renderer )
{
    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );

    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "Rainbow::_commandPool" )

    _commandBuffer = VK_NULL_HANDLE;
}

bool Rainbow::CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer )
{
    VkFenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = 0U;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateFence ( device, &fenceCreateInfo, nullptr, &_presentationFence ),
        "Rainbow::CreatePresentationSyncPrimitive",
        "Can't crate fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "Rainbow::_presentationFence" )

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0U;

    result = renderer.CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_presentationSemaphore ),
        "Rainbow::CreatePresentationSyncPrimitive",
        "Can't create semaphore"
    );

    if ( result )
    {
        AV_REGISTER_SEMAPHORE ( "Rainbow::_presentationSemaphore" )
        return true;
    }

    DestroyPresentationSyncPrimitive ( renderer );
    return false;
}

void Rainbow::DestroyPresentationSyncPrimitive ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _presentationFence )
    {
        vkDestroyFence ( device, _presentationFence, nullptr );
        _presentationFence = VK_NULL_HANDLE;
        AV_UNREGISTER_FENCE ( "Rainbow::_presentationFence" )
    }

    if ( !_presentationSemaphore )
        return;

    vkDestroySemaphore ( device, _presentationSemaphore, nullptr );
    _presentationSemaphore = VK_NULL_HANDLE;
    AV_UNREGISTER_SEMAPHORE ( "Rainbow::_presentationSemaphore" )
}

bool Rainbow::CreateRenderPass ( android_vulkan::Renderer &renderer )
{
    VkAttachmentDescription attachment0;
    attachment0.flags = 0U;
    attachment0.format = renderer.GetSurfaceFormat ();
    attachment0.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment0.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment0.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment0.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment0.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment0.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment0.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0U;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription;
    subpassDescription.flags = 0U;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.inputAttachmentCount = 0U;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.colorAttachmentCount = 1U;
    subpassDescription.pColorAttachments = &colorAttachmentReference;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.pDepthStencilAttachment = nullptr;
    subpassDescription.preserveAttachmentCount = 0U;
    subpassDescription.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0U;
    renderPassCreateInfo.attachmentCount = 1U;
    renderPassCreateInfo.pAttachments = &attachment0;
    renderPassCreateInfo.subpassCount = 1U;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 0U;
    renderPassCreateInfo.pDependencies = nullptr;

    const bool result = renderer.CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassCreateInfo, nullptr, &_renderPass ),
        "Rainbow::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "Rainbow::_renderPass" )
    return true;
}

void Rainbow::DestroyRenderPass ( android_vulkan::Renderer &renderer )
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( renderer.GetDevice (), _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "Rainbow::_renderPass" )
}

} // namespace rainbow
