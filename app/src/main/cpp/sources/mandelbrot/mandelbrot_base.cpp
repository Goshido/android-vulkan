#include <mandelbrot/mandelbrot_base.h>

GX_DISABLE_COMMON_WARNINGS

#include <cmath>

GX_RESTORE_WARNING_STATE

#include <file.h>
#include <vulkan_utils.h>


namespace mandelbrot {

constexpr static const char* VERTEX_SHADER = "shaders/mandelbrot-vs.spv";

constexpr static const char* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr static const char* FRAGMENT_SHADER_ENTRY_POINT = "PS";

MandelbrotBase::MandelbrotBase ( const char* fragmentShaderSpirV ) noexcept:
    _commandBuffer {},
    _commandPool ( VK_NULL_HANDLE ),
    _framebuffers {},
    _pipeline ( VK_NULL_HANDLE ),
    _pipelineLayout ( VK_NULL_HANDLE ),
    _renderPass ( VK_NULL_HANDLE ),
    _renderPassEndedSemaphore ( VK_NULL_HANDLE ),
    _renderTargetAcquiredSemaphore ( VK_NULL_HANDLE ),
    _vertexShader ( VK_NULL_HANDLE ),
    _fragmentShader ( VK_NULL_HANDLE ),
    _fragmentShaderSpirV ( fragmentShaderSpirV )
{
    // NOTHING
}

bool MandelbrotBase::OnFrame ( android_vulkan::Renderer &renderer, double /*deltaTime*/ ) noexcept
{
    uint32_t presentationImageIndex = UINT32_MAX;

    if ( !BeginFrame ( renderer, presentationImageIndex ) )
        return true;

    VkCommandBuffer commandBuffer = _commandBuffer[ static_cast<size_t> ( presentationImageIndex ) ];
    constexpr VkPipelineStageFlags const waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "MandelbrotBase::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return true;

    return EndFrame ( renderer, presentationImageIndex );
}

bool MandelbrotBase::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !CreatePresentationSyncPrimitive ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateCommandPool ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreatePipelineLayout ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    return true;
}

void MandelbrotBase::OnDestroyDevice ( VkDevice device ) noexcept
{
    DestroyPipelineLayout ( device );
    DestroyCommandPool ( device );
    DestroyPresentationSyncPrimitive ( device );
}

bool MandelbrotBase::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !CreateRenderPass ( renderer ) )
        return false;

    if ( !CreateFramebuffers ( renderer ) )
    {
        OnSwapchainDestroyed ( renderer.GetDevice () );
        return false;
    }

    if ( !CreatePipeline ( renderer ) )
    {
        OnSwapchainDestroyed ( renderer.GetDevice () );
        return false;
    }

    return true;
}

void MandelbrotBase::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    DestroyPipeline ( device );
    DestroyFramebuffers ( device );
    DestroyRenderPass ( device );
}

bool MandelbrotBase::IsReady ()
{
    return _renderPassEndedSemaphore != VK_NULL_HANDLE;
}

bool MandelbrotBase::BeginFrame ( android_vulkan::Renderer &renderer, uint32_t &presentationImageIndex )
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

bool MandelbrotBase::EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex )
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

bool MandelbrotBase::CreateCommandPool ( android_vulkan::Renderer &renderer )
{
    VkCommandPoolCreateInfo const commandPoolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
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

void MandelbrotBase::DestroyCommandPool ( VkDevice device )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "MandelbrotBase::_commandPool" )
}

bool MandelbrotBase::CreateFramebuffers ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    const size_t presentationImageCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( presentationImageCount );

    const VkExtent2D& resolution = renderer.GetSurfaceSize ();

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

void MandelbrotBase::DestroyFramebuffers ( VkDevice device )
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

bool MandelbrotBase::CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer )
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

void MandelbrotBase::DestroyPresentationSyncPrimitive ( VkDevice device )
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

bool MandelbrotBase::CreatePipeline ( android_vulkan::Renderer &renderer )
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

    if ( !result )
    {
        DestroyPipeline ( renderer.GetDevice () );
        return false;
    }

    AV_REGISTER_SHADER_MODULE ( "MandelbrotBase::_fragmentShader" )

    VkPipelineShaderStageCreateInfo shaderStageInfo[ 2U ];

    VkPipelineShaderStageCreateInfo& vertexStage = shaderStageInfo[ 0U ];
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pNext = nullptr;
    vertexStage.flags = 0U;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = _vertexShader;
    vertexStage.pName = VERTEX_SHADER_ENTRY_POINT;
    vertexStage.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo& fragmentStage = shaderStageInfo[ 1U ];
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.pNext = nullptr;
    fragmentStage.flags = 0U;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = _fragmentShader;
    fragmentStage.pName = FRAGMENT_SHADER_ENTRY_POINT;
    fragmentStage.pSpecializationInfo = nullptr;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0U;
    vertexInputInfo.vertexBindingDescriptionCount = 0U;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0U;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.pNext = nullptr;
    inputAssemblyInfo.flags = 0U;
    inputAssemblyInfo.primitiveRestartEnable = VK_TRUE;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    const VkExtent2D& surfaceSize = renderer.GetSurfaceSize ();

    VkViewport viewport;
    viewport.x = viewport.y = 0.0F;
    viewport.width = static_cast<float> ( surfaceSize.width );
    viewport.height = static_cast<float> ( surfaceSize.height );
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent = surfaceSize;

    VkPipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.pNext = nullptr;
    viewportInfo.flags = 0U;
    viewportInfo.viewportCount = 1U;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1U;
    viewportInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.pNext = nullptr;
    rasterizationInfo.flags = 0U;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.lineWidth = 1.0F;
    rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.pNext = nullptr;
    multisampleInfo.flags = 0U;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;
    multisampleInfo.pSampleMask = nullptr;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo;
    colorBlendAttachmentInfo.blendEnable = VK_FALSE;

    colorBlendAttachmentInfo.colorWriteMask =
        AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT );

    colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo;
    colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateInfo.pNext = nullptr;
    colorBlendStateInfo.flags = 0U;
    colorBlendStateInfo.attachmentCount = 1U;
    colorBlendStateInfo.pAttachments = &colorBlendAttachmentInfo;
    colorBlendStateInfo.logicOpEnable = VK_FALSE;
    memset ( colorBlendStateInfo.blendConstants, 0, sizeof ( colorBlendStateInfo.blendConstants ) );

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.pNext = nullptr;
    depthStencilInfo.flags = 0U;
    depthStencilInfo.depthTestEnable = VK_FALSE;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;



    VkGraphicsPipelineCreateInfo const pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stageCount = 2U,
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
        .basePipelineHandle = VK_NULL_HANDLE
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( renderer.GetDevice (), VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "MandelbrotBase::CreatePipeline",
        "Can't create pipeline"
    );

    if ( !result )
    {
        DestroyPipeline ( renderer.GetDevice () );
        return false;
    }

    AV_REGISTER_PIPELINE ( "MandelbrotBase::_pipeline" )
    return true;
}

void MandelbrotBase::DestroyPipeline ( VkDevice device )
{
    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "MandelbrotBase::_pipeline" )
    }

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

bool MandelbrotBase::CreateRenderPass ( android_vulkan::Renderer &renderer )
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

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassCreateInfo, nullptr, &_renderPass ),
        "MandelbrotBase::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "MandelbrotBase::_renderPass" )
    return true;
}

void MandelbrotBase::DestroyRenderPass ( VkDevice device )
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "MandelbrotBase::_renderPass" )
}

} // namespace mandelbrot
