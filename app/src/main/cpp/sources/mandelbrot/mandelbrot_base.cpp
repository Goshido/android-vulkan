#include <mandelbrot/mandelbrot_base.hpp>
#include <file.hpp>
#include <vulkan_utils.hpp>


namespace mandelbrot {

namespace {

constexpr char const* VERTEX_SHADER = "shaders/mandelbrot.vs.spv";

constexpr char const* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr char const* FRAGMENT_SHADER_ENTRY_POINT = "PS";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

MandelbrotBase::MandelbrotBase ( char const* fragmentShaderSpirV ) noexcept:
    _fragmentShaderSpirV ( fragmentShaderSpirV )
{
    // NOTHING
}

bool MandelbrotBase::OnFrame ( android_vulkan::Renderer &renderer, double /*deltaTime*/ ) noexcept
{
    CommandInfo &commandInfo = _commandInfo[ _writeCommandInfoIndex ];
    _writeCommandInfoIndex = ++_writeCommandInfoIndex % DUAL_COMMAND_BUFFER;

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &commandInfo._fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "MandelbrotBase::OnFrame",
        "Can't wait fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    uint32_t presentationImageIndex = std::numeric_limits<uint32_t>::max ();

    if ( !BeginFrame ( renderer, commandInfo._acquire, presentationImageIndex ) ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &commandInfo._fence ),
        "mandelbrot::MandelbrotBase::OnFrame",
        "Can't reset fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetCommandPool ( device, commandInfo._pool, 0U ),
        "mandelbrot::MandelbrotBase::OnFrame",
        "Can't reset command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkCommandBuffer commandBuffer = commandInfo._buffer;

    result = RecordCommandBuffer ( renderer,
        commandInfo._buffer,
        _framebufferInfo[ presentationImageIndex ]._framebuffer
    );

    if ( !result ) [[unlikely]]
        return false;

    constexpr VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &commandInfo._acquire,
        .pWaitDstStageMask = &waitFlags,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &_framebufferInfo[ presentationImageIndex ]._renderEnd
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, commandInfo._fence ),
        "mandelbrot::MandelbrotBase::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    return EndFrame ( renderer, presentationImageIndex );
}

bool MandelbrotBase::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreateCommandBuffers ( renderer ) && CreatePipelineLayout ( renderer.GetDevice () );
}

void MandelbrotBase::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    DestroyPipelineLayout ( device );
    DestroyCommandBuffers ( device );
}

bool MandelbrotBase::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreateRenderPass ( renderer ) &&
        CreateFramebuffers ( renderer ) &&
        CreatePipeline ( renderer );
}

void MandelbrotBase::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    DestroyPipeline ( device );
    DestroyFramebuffers ( device );
    DestroyRenderPass ( device );
}

bool MandelbrotBase::IsReady ()
{
    return _renderPass != VK_NULL_HANDLE;
}

bool MandelbrotBase::BeginFrame ( android_vulkan::Renderer &renderer,
    VkSemaphore acquire,
    uint32_t &presentationImageIndex
) noexcept
{
    return android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            UINT64_MAX,
            acquire,
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
        .pWaitSemaphores = &_framebufferInfo[ presentationImageIndex ]._renderEnd,
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

    if ( !result ) [[unlikely]]
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult,
        "MandelbrotBase::EndFrame",
        "Present queue has been failed"
    );
}

bool MandelbrotBase::CreateCommandBuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    _commandInfo.resize ( DUAL_COMMAND_BUFFER );

    VkCommandPoolCreateInfo const commandPoolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkCommandBufferAllocateInfo commandBufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = VK_NULL_HANDLE,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < DUAL_COMMAND_BUFFER; ++i )
    {
        CommandInfo &commandInfo = _commandInfo[ i ];

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateCommandPool ( device, &commandPoolInfo, nullptr, &commandInfo._pool ),
            "MandelbrotBase::CreateCommandBuffers",
            "Can't create command pool"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, commandInfo._pool, VK_OBJECT_TYPE_COMMAND_POOL, "Frame in flight #%zu", i )

        commandBufferInfo.commandPool = commandInfo._pool;

        result = android_vulkan::Renderer::CheckVkResult (
            vkAllocateCommandBuffers ( renderer.GetDevice (), &commandBufferInfo, &commandInfo._buffer ),
            "MandelbrotBase::CreateCommandBuffers",
            "Can't allocate command buffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device,
            commandInfo._buffer,
            VK_OBJECT_TYPE_COMMAND_BUFFER,
            "Frame in flight #%zu", i
        )

        constexpr VkSemaphoreCreateInfo semaphoreInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0U
        };

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &commandInfo._acquire ),
            "MandelbrotBase::CreateCommandBuffers",
            "Can't create render target acquired semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, commandInfo._acquire, VK_OBJECT_TYPE_SEMAPHORE, "Frame in flight #%zu", i )

        constexpr VkFenceCreateInfo fenceInfo
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFence ( device, &fenceInfo, nullptr, &commandInfo._fence ),
            "MandelbrotLUTColor::CreateCommandBuffer",
            "Can't create fence"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, commandInfo._fence, VK_OBJECT_TYPE_FENCE, "Frame in flight #%zu", i )
    }

    return true;
}

void MandelbrotBase::DestroyCommandBuffers ( VkDevice device ) noexcept
{
    for ( auto &commandInfo : _commandInfo )
    {
        if ( commandInfo._pool != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroyCommandPool ( device, commandInfo._pool, nullptr );
            commandInfo._pool = VK_NULL_HANDLE;
        }

        if ( commandInfo._acquire != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroySemaphore ( device, commandInfo._acquire, nullptr );
            commandInfo._acquire = VK_NULL_HANDLE;
        }

        if ( commandInfo._fence == VK_NULL_HANDLE ) [[unlikely]]
            continue;

        vkDestroyFence ( device, commandInfo._fence, nullptr );
    }
}

bool MandelbrotBase::CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    size_t const presentationImageCount = renderer.GetPresentImageCount ();
    _framebufferInfo.resize ( presentationImageCount );

    VkExtent2D const &resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _renderPass,
        .attachmentCount = 1U,
        .pAttachments = nullptr,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    for ( size_t i = 0U; i < presentationImageCount; ++i )
    {
        FramebufferInfo &info = _framebufferInfo[ i ];

        createInfo.pAttachments = &renderer.GetPresentImageView ( i );

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &createInfo, nullptr, &info._framebuffer ),
            "MandelbrotBase::CreateFramebuffers",
            "Can't create framebuffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "Swapchain image #%zu", i )

        constexpr VkSemaphoreCreateInfo semaphoreInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0U
        };

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &info._renderEnd ),
            "MandelbrotBase::CreateFramebuffers",
            "Can't create render end semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._renderEnd, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain image #%zu", i )
    }

    return true;
}

void MandelbrotBase::DestroyFramebuffers ( VkDevice device ) noexcept
{
    for ( auto const fbInfo : _framebufferInfo )
    {
        if ( fbInfo._framebuffer != VK_NULL_HANDLE ) [[likely]]
            vkDestroyFramebuffer ( device, fbInfo._framebuffer, nullptr );

        if ( fbInfo._renderEnd != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroySemaphore ( device, fbInfo._renderEnd, nullptr );
        }
    }

    _framebufferInfo.clear ();
    _framebufferInfo.shrink_to_fit ();
}

bool MandelbrotBase::CreatePipeline ( android_vulkan::Renderer &renderer ) noexcept
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (MandelbrotBase::CreatePipeline)"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkDevice device = renderer.GetDevice ();

    AV_SET_VULKAN_OBJECT_NAME ( device, _vertexShader, VK_OBJECT_TYPE_SHADER_MODULE, VERTEX_SHADER )

    result = renderer.CreateShader ( _fragmentShader,
        _fragmentShaderSpirV,
        "Can't create fragment shader (MandelbrotBase::CreatePipeline)"
    );


    if ( !result ) [[unlikely]]
    {
        DestroyPipeline ( device );
        return false;
    }

    AV_SET_VULKAN_OBJECT_NAME ( device, _fragmentShader, VK_OBJECT_TYPE_SHADER_MODULE, "%s", _fragmentShaderSpirV )

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

    if ( !result ) [[unlikely]]
    {
        DestroyPipeline ( device );
        return false;
    }

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "MandelbrotBase::_pipeline" )
    DestroyShaderModules ( device );
    return true;
}

void MandelbrotBase::DestroyPipeline ( VkDevice device ) noexcept
{
    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
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
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) ),
        .pAttachments = attachments,
        .subpassCount = static_cast<uint32_t> ( std::size ( subpassDescription ) ),
        .pSubpasses = subpassDescription,
        .dependencyCount = 1U,
        .pDependencies = &dependency
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &renderPassCreateInfo, nullptr, &_renderPass ),
        "MandelbrotBase::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderPass, VK_OBJECT_TYPE_RENDER_PASS, "MandelbrotBase::_renderPass" )
    return true;
}

void MandelbrotBase::DestroyRenderPass ( VkDevice device ) noexcept
{
    if ( _renderPass == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
}

void MandelbrotBase::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _vertexShader != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyShaderModule ( device, _vertexShader, nullptr );
        _vertexShader = VK_NULL_HANDLE;
    }

    if ( _fragmentShader == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyShaderModule ( device, _fragmentShader, nullptr );
    _fragmentShader = VK_NULL_HANDLE;
}

} // namespace mandelbrot
