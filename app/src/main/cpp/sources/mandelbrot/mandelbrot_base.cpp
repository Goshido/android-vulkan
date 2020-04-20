#include <mandelbrot/mandelbrot_base.h>
#include <cmath>
#include <file.h>
#include <vulkan_utils.h>


namespace mandelbrot {

constexpr static const char* VERTEX_SHADER = "shaders/mandelbrot-vs.spv";

constexpr static const char* VERTEX_SHADER_ENTRY_POINT = "VS";
constexpr static const char* FRAGMENT_SHADER_ENTRY_POINT = "PS";

bool MandelbrotBase::IsReady ()
{
    return _presentationSemaphore != VK_NULL_HANDLE;
}

bool MandelbrotBase::OnInit ( android_vulkan::Renderer &renderer )
{
    if ( !CreateRenderPass ( renderer ) )
        return false;

    if ( !CreatePresentationSyncPrimitive ( renderer ) )
    {
        DestroyRenderPass ( renderer );
        return false;
    }

    if ( !CreatePipeline ( renderer ) )
    {
        DestroyPresentationSyncPrimitive ( renderer );
        DestroyRenderPass ( renderer );
        return false;
    }

    if ( !CreateCommandPool ( renderer ) )
    {
        DestroyPipeline ( renderer );
        DestroyPresentationSyncPrimitive ( renderer );
        DestroyRenderPass ( renderer );
        return false;
    }

    if ( CreateCommandBuffer ( renderer ) )
        return true;

    DestroyCommandPool ( renderer );
    DestroyPipeline ( renderer );
    DestroyPresentationSyncPrimitive ( renderer );
    DestroyRenderPass ( renderer );

    return false;
}

bool MandelbrotBase::OnFrame ( android_vulkan::Renderer &renderer, double /*deltaTime*/ )
{
    uint32_t presentationImageIndex = UINT32_MAX;

    if ( !BeginFrame ( presentationImageIndex, renderer ) )
        return true;

    VkCommandBuffer commandBuffer = _commandBuffer[ static_cast<size_t> ( presentationImageIndex ) ];

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores = &_presentationSemaphore;

    const bool result = renderer.CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, _presentationFence ),
        "MandelbrotBase::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return true;

    return EndFrame ( presentationImageIndex, renderer );
}

bool MandelbrotBase::OnDestroy ( android_vulkan::Renderer &renderer )
{
    DestroyCommandPool ( renderer );
    DestroyPipeline ( renderer );
    DestroyPresentationSyncPrimitive ( renderer );
    DestroyRenderPass ( renderer );

    return true;
}

MandelbrotBase::MandelbrotBase ( const char* fragmentShaderSpirV ):
    _commandPool ( VK_NULL_HANDLE ),
    _pipeline ( VK_NULL_HANDLE ),
    _renderPass ( VK_NULL_HANDLE ),
    _pipelineLayout ( VK_NULL_HANDLE ),
    _presentationFence ( VK_NULL_HANDLE ),
    _presentationSemaphore ( VK_NULL_HANDLE ),
    _vertexShader ( VK_NULL_HANDLE ),
    _fragmentShader ( VK_NULL_HANDLE ),
    _fragmentShaderSpirV ( fragmentShaderSpirV )
{
    // NOTHING
}

bool MandelbrotBase::BeginFrame ( uint32_t &presentationImageIndex, android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkAcquireNextImageKHR ( device,
            renderer.GetSwapchain (),
            UINT64_MAX,
            VK_NULL_HANDLE,
            _presentationFence,
            &presentationImageIndex
        ),

        "MandelbrotBase::BeginFrame",
        "Can't acquire next image"
    );

    if ( !result )
        return false;

    result = renderer.CheckVkResult ( vkWaitForFences ( device, 1U, &_presentationFence, VK_TRUE, UINT64_MAX ),
        "MandelbrotBase::BeginFrame",
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
        "MandelbrotBase::BeginFrame",
        "Waiting queue idle has been failed"
    );
}

bool MandelbrotBase::EndFrame ( uint32_t presentationImageIndex, android_vulkan::Renderer &renderer )
{
    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    VkPresentInfoKHR presentInfoKHR;
    presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfoKHR.pNext = nullptr;
    presentInfoKHR.waitSemaphoreCount = 1U;
    presentInfoKHR.pWaitSemaphores = &_presentationSemaphore;
    presentInfoKHR.swapchainCount = 1U;
    presentInfoKHR.pSwapchains = &renderer.GetSwapchain ();
    presentInfoKHR.pImageIndices = &presentationImageIndex;
    presentInfoKHR.pResults = &presentResult;

    // Note vkQueuePresentKHR may return VK_SUBOPTIMAL_KHR right before application is minimized.
    const VkResult mainResult = vkQueuePresentKHR ( renderer.GetQueue (), &presentInfoKHR );

    if ( mainResult == VK_SUBOPTIMAL_KHR )
        return true;

    if ( !renderer.CheckVkResult ( mainResult, "Rainbow::EndFrame", "Can't present frame" ) )
        return false;

    return renderer.CheckVkResult ( presentResult, "Rainbow::EndFrame", "Present queue has been failed" );
}

bool MandelbrotBase::CreateCommandPool ( android_vulkan::Renderer &renderer )
{
    VkCommandPoolCreateInfo commandPoolInfo;
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.queueFamilyIndex = renderer.GetQueueFamilyIndex ();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateCommandPool ( device, &commandPoolInfo, nullptr, &_commandPool ),
        "MandelbrotBase::CreateCommandBuffer",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "MandelbrotBase::_commandPool" )
    return true;
}

bool MandelbrotBase::DestroyCommandPool ( android_vulkan::Renderer &renderer )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return true;

    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "MandelbrotBase::_commandPool" )

    return true;
}

bool MandelbrotBase::CreatePresentationSyncPrimitive ( android_vulkan::Renderer &renderer )
{
    VkFenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = 0U;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateFence ( device, &fenceCreateInfo, nullptr, &_presentationFence ),
        "MandelbrotBase::CreatePresentationSyncPrimitive",
        "Can't crate fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "MandelbrotBase::_presentationFence" )

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0U;

    result = renderer.CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreCreateInfo, nullptr, &_presentationSemaphore ),
        "MandelbrotBase::CreatePresentationSyncPrimitive",
        "Can't create semaphore"
    );

    if ( result )
    {
        AV_REGISTER_SEMAPHORE ( "MandelbrotBase::_presentationSemaphore" )
        return true;
    }

    DestroyPresentationSyncPrimitive ( renderer );
    return false;
}

void MandelbrotBase::DestroyPresentationSyncPrimitive ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _presentationFence )
    {
        vkDestroyFence ( device, _presentationFence, nullptr );
        _presentationFence = VK_NULL_HANDLE;
        AV_UNREGISTER_FENCE ( "MandelbrotBase::_presentationFence" )
    }

    if ( !_presentationSemaphore )
        return;

    vkDestroySemaphore ( device, _presentationSemaphore, nullptr );
    _presentationSemaphore = VK_NULL_HANDLE;
    AV_UNREGISTER_SEMAPHORE ( "MandelbrotBase::_presentationSemaphore" )
}

bool MandelbrotBase::CreatePipeline ( android_vulkan::Renderer &renderer )
{
    if ( !CreateShader ( _vertexShader, VERTEX_SHADER, renderer ) )
    {
        android_vulkan::LogError ( "MandelbrotBase::CreatePipeline - Can't create vertex shader." );
        return false;
    }

    AV_REGISTER_SHADER_MODULE ( "MandelbrotBase::_vertexShader" )

    if ( !CreateShader ( _fragmentShader, _fragmentShaderSpirV, renderer ) )
    {
        android_vulkan::LogError ( "MandelbrotBase::CreatePipeline - Can't create fragment shader %s.",
            _fragmentShaderSpirV
        );

        DestroyPipeline ( renderer );
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
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.flags = 0U;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0U;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    pipelineLayoutInfo.setLayoutCount = 0U;
    pipelineLayoutInfo.pSetLayouts = nullptr;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &_pipelineLayout ),
        "MandelbrotBase::CreatePipeline",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "MandelbrotBase::_pipelineLayout" )

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.stageCount = 2U;
    pipelineInfo.pStages = shaderStageInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pColorBlendState = &colorBlendStateInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = renderer.GetPresentRenderPass ();
    pipelineInfo.subpass = 0U;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    result = renderer.CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "MandelbrotBase::CreatePipeline",
        "Can't create pipeline"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE ( "MandelbrotBase::_pipeline" )
    return true;
}

void MandelbrotBase::DestroyPipeline ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "MandelbrotBase::_pipeline" )
    }

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "MandelbrotBase::_pipelineLayout" )
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
        "MandelbrotBase::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "MandelbrotBase::_renderPass" )
    return true;
}

void MandelbrotBase::DestroyRenderPass ( android_vulkan::Renderer &renderer )
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( renderer.GetDevice (), _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "MandelbrotBase::_renderPass" )
}

bool MandelbrotBase::CreateShader ( VkShaderModule &shader,
    const char* shaderFile,
    android_vulkan::Renderer &renderer
) const
{
    android_vulkan::File vertexShader ( shaderFile );

    if ( !vertexShader.LoadContent () )
        return false;

    const std::vector<uint8_t>& spirV = vertexShader.GetContent ();

    VkShaderModuleCreateInfo shaderModuleCreateInfo;
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;
    shaderModuleCreateInfo.flags = 0U;
    shaderModuleCreateInfo.codeSize = spirV.size ();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*> ( spirV.data () );

    return renderer.CheckVkResult (
        vkCreateShaderModule ( renderer.GetDevice (), &shaderModuleCreateInfo, nullptr, &shader ),
        "MandelbrotBase::CreateShader",
        "Can't create shader"
    );
}

} // namespace mandelbrot
