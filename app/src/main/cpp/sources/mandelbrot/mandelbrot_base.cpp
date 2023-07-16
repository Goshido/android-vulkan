#include <mandelbrot/mandelbrot_base.hpp>
#include <file.hpp>
#include <vulkan_utils.hpp>


namespace mandelbrot {

constexpr static char const* VERTEX_SHADER = "shaders/mandelbrot.vs.spv";

constexpr static char const* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr static char const* FRAGMENT_SHADER_ENTRY_POINT = "PS";

//----------------------------------------------------------------------------------------------------------------------

MandelbrotBase::MandelbrotBase ( char const* fragmentShaderSpirV ) noexcept:
    _fragmentShaderSpirV ( fragmentShaderSpirV )
{
    // NOTHING
}

bool MandelbrotBase::OnFrame ( android_vulkan::Renderer &renderer, double /*deltaTime*/ ) noexcept
{
    uint32_t presentationImageIndex = std::numeric_limits<uint32_t>::max ();

    if ( !BeginFrame ( renderer, presentationImageIndex ) )
        return true;

    VkCommandBuffer commandBuffer = _commandBuffer[ static_cast<size_t> ( presentationImageIndex ) ];
    constexpr VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_renderTargetAcquiredSemaphore,
        .pWaitDstStageMask = &waitFlags,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &_renderPassEndedSemaphore
    };

    VkFence fence = _fences[ static_cast<size_t> ( presentationImageIndex ) ];
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "mandelbrot::MandelbrotBase::OnFrame",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &fence ),
        "mandelbrot::MandelbrotBase::OnFrame",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fence ),
        "mandelbrot::MandelbrotBase::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return true;

    return EndFrame ( renderer, presentationImageIndex );
}

bool MandelbrotBase::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreatePresentationSyncPrimitive ( renderer ) &&
        CreateCommandPool ( renderer ) &&
        CreatePipelineLayout ( renderer );
}

void MandelbrotBase::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    DestroyPipelineLayout ( device );
    DestroyCommandPool ( device );
    DestroyPresentationSyncPrimitive ( device );
}

bool MandelbrotBase::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreateRenderPass ( renderer ) &&
        CreateFramebuffers ( renderer ) &&
        CreateFences ( renderer.GetDevice () ) &&
        CreatePipeline ( renderer );
}

void MandelbrotBase::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    DestroyFences ( device );
    DestroyPipeline ( device );
    DestroyFramebuffers ( device );
    DestroyRenderPass ( device );
}

bool MandelbrotBase::IsReady ()
{
    return _renderPassEndedSemaphore != VK_NULL_HANDLE;
}

bool MandelbrotBase::BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationImageIndex ) noexcept
{
    return android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            UINT64_MAX,
            _renderTargetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &presentationImageIndex
        ),

        "MandelbrotBase::BeginFrame",
        "Can't acquire next image"
    );
}

bool MandelbrotBase::EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex ) noexcept
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
        .pImageIndices = &presentationImageIndex,
        .pResults = &presentResult
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkQueuePresentKHR ( renderer.GetQueue (), &presentInfoKHR ),
        "MandelbrotBase::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult,
        "MandelbrotBase::EndFrame",
        "Present queue has been failed"
    );
}

bool MandelbrotBase::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const commandPoolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &commandPoolInfo, nullptr, &_commandPool ),
        "MandelbrotBase::CreateCommandBuffer",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "MandelbrotBase::_commandPool" )
    return true;
}

void MandelbrotBase::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "MandelbrotBase::_commandPool" )
}

bool MandelbrotBase::CreateFences ( VkDevice device ) noexcept
{
    size_t const count = _framebuffers.size ();
    _fences.clear ();
    _fences.reserve ( count );

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for ( size_t i = 0U; i < count; ++i )
    {
        VkFence fence;

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFence ( device, &fenceInfo, nullptr, &fence ),
            "mandelbrot::MandelbrotBase::CreateFences",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "mandelbrot::MandelbrotBase::_fence" )
        _fences.push_back ( fence );
    }

    return true;
}

void MandelbrotBase::DestroyFences ( VkDevice device ) noexcept
{
    for ( auto fence : _fences )
    {
        vkDestroyFence ( device, fence, nullptr );
        AV_UNREGISTER_FENCE ( "mandelbrot::MandelbrotBase::_fence" )
    }

    _fences.clear ();
    _fences.shrink_to_fit ();
}

bool MandelbrotBase::CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    size_t const presentationImageCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( presentationImageCount );

    VkExtent2D const &resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0U;
    createInfo.renderPass = _renderPass;
    createInfo.width = resolution.width;
    createInfo.height = resolution.height;
    createInfo.attachmentCount = 1U;
    createInfo.layers = 1U;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    for ( size_t i = 0U; i < presentationImageCount; ++i )
    {
        createInfo.pAttachments = &renderer.GetPresentImageView ( i );

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &createInfo, nullptr, &framebuffer ),
            "MandelbrotBase::CreateFramebuffers",
            "Can't create framebuffer"
        );

        if ( !result )
            return false;

        _framebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "MandelbrotBase::_framebuffers" )
    }

    return true;
}

void MandelbrotBase::DestroyFramebuffers ( VkDevice device ) noexcept
{
    if ( _framebuffers.empty () )
        return;

    for ( auto const framebuffer : _framebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "MandelbrotBase::_framebuffers" )
    }

    _framebuffers.clear ();
    _framebuffers.shrink_to_fit ();
}

bool MandelbrotBase::CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0U;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_renderTargetAcquiredSemaphore ),
        "MandelbrotBase::CreatePresentationSyncPrimitive",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
    {
        DestroyPresentationSyncPrimitive ( renderer.GetDevice () );
        return false;
    }

    AV_REGISTER_SEMAPHORE ( "MandelbrotBase::_renderTargetAcquiredSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_renderPassEndedSemaphore ),
        "MandelbrotBase::CreatePresentationSyncPrimitive",
        "Can't create render pass ended semaphore"
    );

    if ( !result )
    {
        DestroyPresentationSyncPrimitive ( renderer.GetDevice () );
        return false;
    }

    AV_REGISTER_SEMAPHORE ( "MandelbrotBase::_renderPassEndedSemaphore" )
    return true;
}

void MandelbrotBase::DestroyPresentationSyncPrimitive ( VkDevice device ) noexcept
{
    if ( _renderPassEndedSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _renderPassEndedSemaphore, nullptr );
        _renderPassEndedSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "MandelbrotBase::_renderPassEndedSemaphore" )
    }

    if ( _renderTargetAcquiredSemaphore == VK_NULL_HANDLE )
        return;

    vkDestroySemaphore ( device, _renderTargetAcquiredSemaphore, nullptr );
    _renderTargetAcquiredSemaphore = VK_NULL_HANDLE;
    AV_UNREGISTER_SEMAPHORE ( "MandelbrotBase::_renderTargetAcquiredSemaphore" )
}

bool MandelbrotBase::CreatePipeline ( android_vulkan::Renderer &renderer ) noexcept
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (MandelbrotBase::CreatePipeline)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "MandelbrotBase::_vertexShader" )

    result = renderer.CreateShader ( _fragmentShader,
        _fragmentShaderSpirV,
        "Can't create fragment shader (MandelbrotBase::CreatePipeline)"
    );

    VkDevice device = renderer.GetDevice ();

    if ( !result )
    {
        DestroyPipeline ( device );
        return false;
    }

    AV_REGISTER_SHADER_MODULE ( "MandelbrotBase::_fragmentShader" )

    VkPipelineShaderStageCreateInfo const shaderStageInfo[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0U,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = _vertexShader,
            .pName = VERTEX_SHADER_ENTRY_POINT,
            .pSpecializationInfo = nullptr
        },

        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0U,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = _fragmentShader,
            .pName = FRAGMENT_SHADER_ENTRY_POINT,
            .pSpecializationInfo = nullptr
        }
    };

    constexpr VkPipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .vertexBindingDescriptionCount = 0U,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0U,
        .pVertexAttributeDescriptions = nullptr
    };

    constexpr VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .primitiveRestartEnable = VK_TRUE
    };

    VkExtent2D const &surfaceSize = renderer.GetSurfaceSize ();

    VkViewport const viewport
    {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float> ( surfaceSize.width ),
        .height = static_cast<float> ( surfaceSize.height ),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };

    VkRect2D const scissor
    {
        .offset =
        {
            .x = 0,
            .y = 0
        },

        .extent = surfaceSize
    };

    VkPipelineViewportStateCreateInfo const viewportInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .viewportCount = 1U,
        .pViewports = &viewport,
        .scissorCount = 1U,
        .pScissors = &scissor
    };

    constexpr VkPipelineRasterizationStateCreateInfo rasterizationInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0F,
        .depthBiasClamp = 0.0F,
        .depthBiasSlopeFactor = 0.0F,
        .lineWidth = 1.0F
    };

    constexpr VkPipelineMultisampleStateCreateInfo multisampleInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0F,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    constexpr VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo
    {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,

        .colorWriteMask =
            AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT )
    };

    VkPipelineColorBlendStateCreateInfo const colorBlendStateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1U,
        .pAttachments = &colorBlendAttachmentInfo,
        .blendConstants = { 0.0F, 0.0F, 0.0F, 0.0F }
    };

    constexpr VkPipelineDepthStencilStateCreateInfo depthStencilInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_ALWAYS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,

        .front =
        {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = std::numeric_limits<uint32_t>::max (),
            .writeMask = 0U,
            .reference = std::numeric_limits<uint32_t>::max ()
        },

        .back =
        {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = std::numeric_limits<uint32_t>::max (),
            .writeMask = 0U,
            .reference = std::numeric_limits<uint32_t>::max ()
        },

        .minDepthBounds = 0.0F,
        .maxDepthBounds = 0.0F
    };

    VkGraphicsPipelineCreateInfo const pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stageCount = static_cast<uint32_t> ( std::size ( shaderStageInfo ) ),
        .pStages = shaderStageInfo,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterizationInfo,
        .pMultisampleState = &multisampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlendStateInfo,
        .pDynamicState = nullptr,
        .layout = _pipelineLayout,
        .renderPass = _renderPass,
        .subpass = 0U,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "MandelbrotBase::CreatePipeline",
        "Can't create pipeline"
    );

    if ( !result )
    {
        DestroyPipeline ( device );
        return false;
    }

    AV_REGISTER_PIPELINE ( "MandelbrotBase::_pipeline" )
    DestroyShaderModules ( device );
    return true;
}

void MandelbrotBase::DestroyPipeline ( VkDevice device ) noexcept
{
    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "MandelbrotBase::_pipeline" )
    }

    DestroyShaderModules ( device );
}

bool MandelbrotBase::CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept
{
    VkAttachmentDescription const attachments[]
    {
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
        }
    };

    constexpr static VkAttachmentReference const colorAttachmentReference[]
    {
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    constexpr VkSubpassDescription const subpassDescription[]
    {
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0U,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( colorAttachmentReference ) ),
            .pColorAttachments = colorAttachmentReference,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        }
    };

    VkRenderPassCreateInfo const renderPassCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) ),
        .pAttachments = attachments,
        .subpassCount = static_cast<uint32_t> ( std::size ( subpassDescription ) ),
        .pSubpasses = subpassDescription,
        .dependencyCount = 0U,
        .pDependencies = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassCreateInfo, nullptr, &_renderPass ),
        "MandelbrotBase::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "MandelbrotBase::_renderPass" )
    return true;
}

void MandelbrotBase::DestroyRenderPass ( VkDevice device ) noexcept
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "MandelbrotBase::_renderPass" )
}

void MandelbrotBase::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _vertexShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, _vertexShader, nullptr );
        _vertexShader = VK_NULL_HANDLE;
        AV_UNREGISTER_SHADER_MODULE ( "MandelbrotBase::_vertexShader" )
    }

    if ( _fragmentShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _fragmentShader, nullptr );
    _fragmentShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "MandelbrotBase::_fragmentShader" )
}

} // namespace mandelbrot
