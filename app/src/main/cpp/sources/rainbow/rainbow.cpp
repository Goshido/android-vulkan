#include <rainbow/rainbow.h>

GX_DISABLE_COMMON_WARNINGS

#include <cmath>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace rainbow {

Rainbow::Rainbow ():
    _commandBuffers {},
    _commandPool ( VK_NULL_HANDLE ),
    _framebuffers {},
    _renderPass ( VK_NULL_HANDLE ),
    _renderPassEndedSemaphore ( VK_NULL_HANDLE ),
    _renderTargetAcquiredSemaphore ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool Rainbow::IsReady ()
{
    return _renderPassEndedSemaphore != VK_NULL_HANDLE;
}

bool Rainbow::OnInit ( android_vulkan::Renderer &renderer )
{
    if ( !CreateRenderPass ( renderer ) )
        return false;

    if ( !CreateFramebuffers ( renderer ) )
    {
        DestroyFramebuffers ( renderer );
        return false;
    }

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
    uint32_t framebufferIndex = UINT32_MAX;

    if ( !BeginFrame ( framebufferIndex, renderer ) )
        return true;

    const CommandContext& commandContext = _commandBuffers[ static_cast<size_t> ( framebufferIndex ) ];
    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkWaitForFences ( device, 1U, &commandContext.second, VK_TRUE, UINT64_MAX ),
        "Rainbow::OnFrame",
        "Can't wait command buffer fence"
    );

    if ( !result )
        return false;

    result = renderer.CheckVkResult ( vkResetFences ( device, 1U, &commandContext.second ),
        "Rainbow::OnFrame",
        "Can't reset command buffer fence"
    );

    if ( !result )
        return false;

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandContext.first, &commandBufferBeginInfo ),
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

    VkClearValue colorClearValue;
    colorClearValue.color.float32[ 0U ] = colorSolver ( 0.0F );
    colorClearValue.color.float32[ 1U ] = colorSolver ( CIRCLE_THIRD );
    colorClearValue.color.float32[ 2U ] = colorSolver ( CIRCLE_TWO_THIRD );
    colorClearValue.color.float32[ 3U ] = 1.0F;

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffers[ framebufferIndex ];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();
    renderPassBeginInfo.clearValueCount = 1U;
    renderPassBeginInfo.pClearValues = &colorClearValue;

    vkCmdBeginRenderPass ( commandContext.first, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdEndRenderPass ( commandContext.first );

    result = renderer.CheckVkResult ( vkEndCommandBuffer ( commandContext.first ),
        "Rainbow::OnFrame",
        "Can't end command buffer"
    );

    if ( !result )
        return true;

    constexpr const VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1U;
    submitInfo.pWaitSemaphores = &_renderTargetAcquiredSemaphore;
    submitInfo.pWaitDstStageMask = &waitFlags;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &commandContext.first;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores = &_renderPassEndedSemaphore;

    result = renderer.CheckVkResult ( vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, commandContext.second ),
        "Rainbow::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return true;

    return EndFrame ( framebufferIndex, renderer );
}

bool Rainbow::OnDestroy ( android_vulkan::Renderer &renderer )
{
    const bool result = renderer.CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "Rainbow::OnDestroy",
        "Can't wait queue idle"
    );

    if ( !result )
        return false;

    DestroyPresentationSyncPrimitive ( renderer );
    DestroyCommandBuffer ( renderer );
    DestroyFramebuffers ( renderer );
    DestroyRenderPass ( renderer );

    return true;
}

bool Rainbow::BeginFrame ( uint32_t &presentationFramebufferIndex, android_vulkan::Renderer &renderer )
{
    return renderer.CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            UINT64_MAX,
            _renderTargetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &presentationFramebufferIndex
        ),

        "Rainbow::BeginFrame",
        "Can't acquire next image"
    );
}

bool Rainbow::EndFrame ( uint32_t presentationFramebufferIndex, android_vulkan::Renderer &renderer )
{
    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    VkPresentInfoKHR presentInfoKHR;
    presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfoKHR.pNext = nullptr;
    presentInfoKHR.waitSemaphoreCount = 1U;
    presentInfoKHR.pWaitSemaphores = &_renderPassEndedSemaphore;
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

    const size_t framebufferCount = renderer.GetPresentImageCount ();
    _commandBuffers.reserve ( framebufferCount );

    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t> ( framebufferCount );
    commandBufferAllocateInfo.commandPool = _commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    std::vector<VkCommandBuffer> commandBuffers ( framebufferCount );

    result = renderer.CheckVkResult (
        vkAllocateCommandBuffers ( device, &commandBufferAllocateInfo, commandBuffers.data () ),
        "Rainbow::CreateCommandBuffer",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence = VK_NULL_HANDLE;

    for ( const auto commandBuffer : commandBuffers )
    {
        result = renderer.CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &fence ),
            "Rainbow::CreateCommandBuffer",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "Rainbow::_commandBuffers::_fence" );
        _commandBuffers.push_back ( std::make_pair ( commandBuffer, fence ) );
    }

    return true;
}

void Rainbow::DestroyCommandBuffer ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( !_commandBuffers.empty () )
    {
        for ( const auto& item : _commandBuffers )
        {
            vkDestroyFence ( device, item.second, nullptr );
            AV_UNREGISTER_FENCE ( "Rainbow::_commandBuffers::_fence" );
        }

        _commandBuffers.clear ();
    }

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "Rainbow::_commandPool" )
}

bool Rainbow::CreateFramebuffers ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();
    const size_t framebufferCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( framebufferCount );

    const VkExtent2D& resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.flags = 0U;
    createInfo.pNext = nullptr;
    createInfo.renderPass = _renderPass;
    createInfo.width = resolution.width;
    createInfo.height = resolution.height;
    createInfo.layers = 1U;
    createInfo.attachmentCount = 1U;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        createInfo.pAttachments = &renderer.GetPresentImageView ( i );

        const bool result = renderer.CheckVkResult (
            vkCreateFramebuffer ( device, &createInfo, nullptr, &framebuffer ),
            "Rainbow::CreateFramebuffers",
            "Can't create framebuffer"
        );

        if ( !result )
            return false;

        _framebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "Rainbow::_framebuffers" )
    }

    return true;
}

void Rainbow::DestroyFramebuffers ( android_vulkan::Renderer &renderer )
{
    if ( _framebuffers.empty () )
        return;

    const VkDevice device = renderer.GetDevice ();

    for ( const auto framebuffer : _framebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "Rainbow::_framebuffers" )
    }

    _framebuffers.clear ();
}

bool Rainbow::CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0U;

    bool result = renderer.CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_renderTargetAcquiredSemaphore ),
        "Rainbow::CreatePresentationSyncPrimitive",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
    {
        DestroyPresentationSyncPrimitive ( renderer );
        return false;
    }

    AV_REGISTER_SEMAPHORE ( "Rainbow::_renderTargetAcquiredSemaphore" )

    result = renderer.CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_renderPassEndedSemaphore ),
        "Rainbow::CreatePresentationSyncPrimitive",
        "Can't create render pass ended semaphore"
    );

    if ( !result )
    {
        DestroyPresentationSyncPrimitive ( renderer );
        return false;
    }

    AV_REGISTER_SEMAPHORE ( "Rainbow::_renderPassEndedSemaphore" )
    return true;
}

void Rainbow::DestroyPresentationSyncPrimitive ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _renderTargetAcquiredSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _renderTargetAcquiredSemaphore, nullptr );
        _renderTargetAcquiredSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "Rainbow::_renderTargetAcquiredSemaphore" )
    }

    if ( _renderPassEndedSemaphore == VK_NULL_HANDLE )
        return;

    vkDestroySemaphore ( device, _renderPassEndedSemaphore, nullptr );
    _renderPassEndedSemaphore = VK_NULL_HANDLE;
    AV_UNREGISTER_SEMAPHORE ( "Rainbow::_renderPassEndedSemaphore" )
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
