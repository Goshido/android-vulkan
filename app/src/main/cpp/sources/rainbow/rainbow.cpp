#include <rainbow/rainbow.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cmath>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.hpp>


namespace rainbow {

bool Rainbow::IsReady () noexcept
{
    return _renderPassEndedSemaphore != VK_NULL_HANDLE;
}

bool Rainbow::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, UINT64_MAX ),
        "Rainbow::OnFrame",
        "Can't wait command buffer fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetFences ( device, 1U, &_fence ),
        "Rainbow::OnFrame",
        "Can't reset command buffer fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _commandPool, 0U ),
        "Rainbow::OnFrame",
        "Can't reset command pool"
    );

    if ( !result )
        return false;

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkBeginCommandBuffer ( _commandBuffer, &commandBufferBeginInfo ),
        "Rainbow::OnFrame",
        "Can't begin command buffer"
    );

    if ( !result )
        return true;

    uint32_t framebufferIndex = UINT32_MAX;

    if ( !BeginFrame ( renderer, framebufferIndex ) )
        return true;

    constexpr float CIRCLE_THIRD = GX_MATH_DOUBLE_PI / 3.0F;
    constexpr float CIRCLE_TWO_THIRD = 2.0F * CIRCLE_THIRD;
    constexpr float RECOLOR_SPEED = 0.777F;

    static float colorFactor = 0.0F;
    colorFactor += RECOLOR_SPEED * static_cast<float> ( deltaTime );

    auto const colorSolver = [ & ] ( float offset ) -> float {
        return std::sin ( colorFactor + offset ) * 0.5F + 0.5F;
    };

    VkClearValue const colorClearValue
    {
        .color
        {
            .float32 { colorSolver ( 0.0F ), colorSolver ( CIRCLE_THIRD ), colorSolver ( CIRCLE_TWO_THIRD ), 1.0F }
        }
    };

    VkRenderPassBeginInfo const renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = _renderPass,
        .framebuffer = _framebuffers[ framebufferIndex ],

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
        .pClearValues = &colorClearValue
    };

    vkCmdBeginRenderPass ( _commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdEndRenderPass ( _commandBuffer );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _commandBuffer ),
        "Rainbow::OnFrame",
        "Can't end command buffer"
    );

    if ( !result )
        return true;

    constexpr VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_renderTargetAcquiredSemaphore,
        .pWaitDstStageMask = &waitFlags,
        .commandBufferCount = 1U,
        .pCommandBuffers = &_commandBuffer,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &_renderPassEndedSemaphore
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, _fence ),
        "Rainbow::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return true;

    return EndFrame ( renderer, framebufferIndex );
}

bool Rainbow::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreatePresentationSyncPrimitive ( renderer );
}

void Rainbow::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    DestroyPresentationSyncPrimitive ( renderer.GetDevice () );
}

bool Rainbow::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreateRenderPass ( renderer ) && CreateFramebuffers ( renderer ) && CreateCommandBuffer ( renderer );
}

void Rainbow::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    DestroyCommandBuffer ( device );
    DestroyFramebuffers ( device );
    DestroyRenderPass ( device );
}

bool Rainbow::BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationFramebufferIndex ) noexcept
{
    return android_vulkan::Renderer::CheckVkResult (
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

bool Rainbow::EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationFramebufferIndex ) noexcept
{
    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    VkPresentInfoKHR const presentInfoKHR
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_renderPassEndedSemaphore,
        .swapchainCount = 1U,
        .pSwapchains = &renderer.GetSwapchain (),
        .pImageIndices = &presentationFramebufferIndex,
        .pResults = &presentResult
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkQueuePresentKHR ( renderer.GetQueue (), &presentInfoKHR ),
        "Rainbow::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult,
        "Rainbow::EndFrame",
        "Present queue has been failed"
    );
}

bool Rainbow::CreateCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const commandPoolCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &commandPoolCreateInfo, nullptr, &_commandPool ),
        "Rainbow::CreateCommandBuffer",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "Rainbow::_commandPool" )

    VkCommandBufferAllocateInfo const commandBufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &commandBufferAllocateInfo, &_commandBuffer ),
        "Rainbow::CreateCommandBuffer",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFence ( device, &fenceInfo, nullptr, &_fence ),
        "Rainbow::CreateCommandBuffer",
        "Can't create fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "Rainbow::_fence" )
    return true;
}

void Rainbow::DestroyCommandBuffer ( VkDevice device ) noexcept
{
    if ( _fence != VK_NULL_HANDLE )
    {
        vkDestroyFence ( device, _fence, nullptr );
        AV_UNREGISTER_FENCE ( "Rainbow::_fence" )
        _fence = VK_NULL_HANDLE;
    }

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "Rainbow::_commandPool" )
}

bool Rainbow::CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( framebufferCount );

    VkExtent2D const &resolution = renderer.GetSurfaceSize ();

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

        bool const result = android_vulkan::Renderer::CheckVkResult (
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

void Rainbow::DestroyFramebuffers ( VkDevice device ) noexcept
{
    if ( _framebuffers.empty () )
        return;

    for ( const auto framebuffer : _framebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "Rainbow::_framebuffers" )
    }

    _framebuffers.clear ();
}

bool Rainbow::CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    constexpr VkSemaphoreCreateInfo const semaphoreCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_renderTargetAcquiredSemaphore ),
        "Rainbow::CreatePresentationSyncPrimitive",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
    {
        DestroyPresentationSyncPrimitive ( renderer.GetDevice () );
        return false;
    }

    AV_REGISTER_SEMAPHORE ( "Rainbow::_renderTargetAcquiredSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_renderPassEndedSemaphore ),
        "Rainbow::CreatePresentationSyncPrimitive",
        "Can't create render pass ended semaphore"
    );

    if ( !result )
    {
        DestroyPresentationSyncPrimitive ( renderer.GetDevice () );
        return false;
    }

    AV_REGISTER_SEMAPHORE ( "Rainbow::_renderPassEndedSemaphore" )
    return true;
}

void Rainbow::DestroyPresentationSyncPrimitive ( VkDevice device ) noexcept
{
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

bool Rainbow::CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept
{
    VkAttachmentDescription const attachment0
    {
        .flags = 0U,
        .format = renderer.GetSurfaceFormat (),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    constexpr VkAttachmentReference colorAttachmentReference
    {
        .attachment = 0U,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription const subpassDescription
    {
        .flags = 0U,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0U,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1U,
        .pColorAttachments = &colorAttachmentReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0U,
        .pPreserveAttachments = nullptr
    };

    constexpr VkSubpassDependency dependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0U,
        .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_NONE,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    };

    VkRenderPassCreateInfo const renderPassCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = 1U,
        .pAttachments = &attachment0,
        .subpassCount = 1U,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1U,
        .pDependencies = &dependency
    };

    VkDevice device = renderer.GetDevice ();

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &renderPassCreateInfo, nullptr, &_renderPass ),
        "Rainbow::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderPass, VK_OBJECT_TYPE_RENDER_PASS, "Rainbow::_renderPass" )
    return true;
}

void Rainbow::DestroyRenderPass ( VkDevice device ) noexcept
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
}

} // namespace rainbow
