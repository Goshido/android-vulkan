#include <rainbow/rainbow.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cmath>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.hpp>


namespace rainbow {

bool Rainbow::IsReady () noexcept
{
    return !_swapchainInfo.empty ();
}

bool Rainbow::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    FrameInFlight const &fif = _framesInFlight[ _writeFrameIdndex++ ];
    _writeFrameIdndex %= FRAMES_IN_FLIGHT;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &fif._fence, VK_TRUE, UINT64_MAX ),
        "Rainbow::OnFrame",
        "Can't wait command buffer fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    uint32_t swapchainImageIdx;

    if ( !BeginFrame ( renderer, swapchainImageIdx, fif._acquire ) ) [[unlikely]]
        return false;

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetFences ( device, 1U, &fif._fence ),
        "Rainbow::OnFrame",
        "Can't reset command buffer fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, fif._pool, 0U ),
        "Rainbow::OnFrame",
        "Can't reset command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    SwapchainInfo const &swapchainInfo = _swapchainInfo[ static_cast<size_t> ( swapchainImageIdx ) ];

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkBeginCommandBuffer ( fif._buffer, &commandBufferBeginInfo ),
        "Rainbow::OnFrame",
        "Can't begin command buffer"
    );

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

    _renderPassBeginInfo.pClearValues = &colorClearValue;
    _renderPassBeginInfo.framebuffer = swapchainInfo._framebuffer;

    VkCommandBuffer commandBuffer = fif._buffer;

    vkCmdBeginRenderPass ( commandBuffer, &_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdEndRenderPass ( commandBuffer );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "Rainbow::OnFrame",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    constexpr VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &fif._acquire,
        .pWaitDstStageMask = &waitFlags,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &swapchainInfo._renderEnd
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fif._fence ),
        "Rainbow::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    return EndFrame ( renderer, swapchainImageIdx, swapchainInfo._renderEnd );
}

bool Rainbow::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const commandPoolCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkCommandBufferAllocateInfo commandBufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = VK_NULL_HANDLE,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    constexpr VkSemaphoreCreateInfo const semaphoreCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < FRAMES_IN_FLIGHT; ++i )
    {
        FrameInFlight &info = _framesInFlight[ i ];

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateCommandPool ( device, &commandPoolCreateInfo, nullptr, &info._pool ),
            "Rainbow::OnInitDevice",
            "Can't create command pool"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._pool, VK_OBJECT_TYPE_COMMAND_POOL, "Frame in flight #%zu", i )

        commandBufferAllocateInfo.commandPool = info._pool;

        result = android_vulkan::Renderer::CheckVkResult (
            vkAllocateCommandBuffers ( device, &commandBufferAllocateInfo, &info._buffer ),
            "Rainbow::OnInitDevice",
            "Can't allocate command buffers"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._buffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "Frame in flight #%zu", i )

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &info._acquire ),
            "Rainbow::OnInitDevice",
            "Can't create render target acquired semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._acquire, VK_OBJECT_TYPE_SEMAPHORE, "Frame in flight #%zu", i )

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFence ( device, &fenceInfo, nullptr, &info._fence ),
            "Rainbow::OnInitDevice",
            "Can't create fence"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._fence, VK_OBJECT_TYPE_FENCE, "Frame in flight #%zu", i )
    }

    return true;
}

void Rainbow::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    for ( auto &info : _framesInFlight )
    {
        if ( info._fence != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroyFence ( device, info._fence, nullptr );
            info._fence = VK_NULL_HANDLE;
        }

        if ( info._acquire != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroySemaphore ( device, info._acquire, nullptr );
            info._acquire = VK_NULL_HANDLE;
        }

        if ( info._pool == VK_NULL_HANDLE ) [[unlikely]]
            return;

        vkDestroyCommandPool ( device, info._pool, nullptr );
        info._pool = VK_NULL_HANDLE;
        info._buffer = VK_NULL_HANDLE;
    }
}

bool Rainbow::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreateRenderPass ( renderer ) && CreateSwapchainInfo ( renderer );
}

void Rainbow::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    DestroySwapchainInfo ( device );
    DestroyRenderPass ( device );
}

bool Rainbow::BeginFrame ( android_vulkan::Renderer &renderer,
    uint32_t &swapchainImageIndex,
    VkSemaphore acquire
) noexcept
{
    return android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            UINT64_MAX,
            acquire,
            VK_NULL_HANDLE,
            &swapchainImageIndex
        ),

        "Rainbow::BeginFrame",
        "Can't acquire next image"
    );
}

bool Rainbow::EndFrame ( android_vulkan::Renderer &renderer,
    uint32_t swapchainImageIndex,
    VkSemaphore renderEnd
) noexcept
{
    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    VkPresentInfoKHR const presentInfoKHR
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &renderEnd,
        .swapchainCount = 1U,
        .pSwapchains = &renderer.GetSwapchain (),
        .pImageIndices = &swapchainImageIndex,
        .pResults = &presentResult
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkQueuePresentKHR ( renderer.GetQueue (), &presentInfoKHR ),
        "Rainbow::EndFrame",
        "Can't present frame"
    );

    if ( !result ) [[unlikely]]
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult,
        "Rainbow::EndFrame",
        "Present queue has been failed"
    );
}

bool Rainbow::CreateSwapchainInfo ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    size_t const imageCount = renderer.GetPresentImageCount ();

    _swapchainInfo.resize ( imageCount );
    SwapchainInfo* swapchainInfo = _swapchainInfo.data ();

    VkExtent2D const &resolution = renderer.GetSurfaceSize ();
    _renderPassBeginInfo.renderArea.extent = resolution;

    VkFramebufferCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _renderPassBeginInfo.renderPass,
        .attachmentCount = 1U,
        .pAttachments = nullptr,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U,
    };

    constexpr VkSemaphoreCreateInfo const semaphoreCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    for ( size_t i = 0U; i < imageCount; ++i )
    {
        SwapchainInfo &info = swapchainInfo[ i ];

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &info._renderEnd ),
            "Rainbow::CreateSwapchainInfo",
            "Can't create render target acquired semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._renderEnd, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain image #%zu", i )

        createInfo.pAttachments = &renderer.GetPresentImageView ( i );

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &createInfo, nullptr, &info._framebuffer ),
            "Rainbow::CreateSwapchainInfo",
            "Can't create framebuffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "Swapchain image #%zu", i )
    }

    return true;
}

void Rainbow::DestroySwapchainInfo ( VkDevice device ) noexcept
{
    for ( auto const &info : _swapchainInfo )
    {
        if ( info._framebuffer != VK_NULL_HANDLE ) [[likely]]
            vkDestroyFramebuffer ( device, info._framebuffer, nullptr );

        if ( info._renderEnd != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroySemaphore ( device, info._renderEnd, nullptr );
        }
    }

    _swapchainInfo.clear ();
    _swapchainInfo.shrink_to_fit ();

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
        vkCreateRenderPass ( device, &renderPassCreateInfo, nullptr, &_renderPassBeginInfo.renderPass ),
        "Rainbow::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _renderPassBeginInfo.renderPass,
        VK_OBJECT_TYPE_RENDER_PASS,
        "Rainbow::_renderPass"
    )

    return true;
}

void Rainbow::DestroyRenderPass ( VkDevice device ) noexcept
{
    if ( _renderPassBeginInfo.renderPass == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyRenderPass ( device, _renderPassBeginInfo.renderPass, nullptr );
    _renderPassBeginInfo.renderPass = VK_NULL_HANDLE;
}

} // namespace rainbow
