#include <pbr/texture_present_program.h>


namespace pbr {

constexpr static char const* VERTEX_SHADER = "shaders/screen-quad-vs.spv";
constexpr static char const* FRAGMENT_SHADER = "shaders/texture-present-ps.spv";

constexpr static size_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr static size_t STAGE_COUNT = 2U;

//----------------------------------------------------------------------------------------------------------------------

TexturePresentProgram::TexturePresentProgram () noexcept:
    Program ( "pbr::TexturePresentProgram" )
{
    // NOTHING
}

bool TexturePresentProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
) noexcept
{
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    VkPipelineColorBlendAttachmentState attachmentInfo[ COLOR_RENDER_TARGET_COUNT ];
    VkPipelineColorBlendStateCreateInfo blendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkRect2D scissorDescription;
    VkPipelineShaderStageCreateInfo stageInfo[ STAGE_COUNT ];
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkViewport viewportDescription;
    VkPipelineViewportStateCreateInfo viewportInfo;

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.stageCount = std::size ( stageInfo );

    VkDevice device = renderer.GetDevice ();

    if ( !InitShaderInfo ( renderer, pipelineInfo.pStages, stageInfo ) )
    {
        Destroy ( device );
        return false;
    }

    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo, nullptr, nullptr );
    pipelineInfo.pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo );
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = InitViewportInfo ( viewportInfo, scissorDescription, viewportDescription, viewport );
    pipelineInfo.pRasterizationState = InitRasterizationInfo ( rasterizationInfo );
    pipelineInfo.pMultisampleState = InitMultisampleInfo ( multisampleInfo );
    pipelineInfo.pDepthStencilState = InitDepthStencilInfo ( depthStencilInfo );
    pipelineInfo.pColorBlendState = InitColorBlendInfo ( blendInfo, attachmentInfo );
    pipelineInfo.pDynamicState = nullptr;

    if ( !InitLayout ( device, pipelineInfo.layout ) )
    {
        Destroy ( device );
        return false;
    }

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::TexturePresentProgram::Init",
        "Can't create pipeline"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_PIPELINE ( "pbr::TexturePresentProgram::_pipeline" )
    DestroyShaderModules ( device );
    return true;
}

void TexturePresentProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "pbr::TexturePresentProgram::_pipeline" )
    }

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "pbr::TexturePresentProgram::_pipelineLayout" )
    }

    _descriptorSetLayout.Destroy ( device );
    DestroyShaderModules ( device );
}

Program::DescriptorSetInfo const& TexturePresentProgram::GetResourceInfo () const noexcept
{
    static DescriptorSetInfo const null;
    return null;
}

void TexturePresentProgram::SetData ( VkCommandBuffer commandBuffer,
    VkDescriptorSet set,
    GXMat4 const &transform
) const noexcept
{
    vkCmdPushConstants ( commandBuffer,
        _pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0U,
        sizeof ( PushConstants ),
        &transform
    );

    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &set,
        0U,
        nullptr
    );
}

VkPipelineColorBlendStateCreateInfo const* TexturePresentProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    attachments->blendEnable = VK_FALSE;

    attachments->colorWriteMask =
        AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT );

    attachments->alphaBlendOp = VK_BLEND_OP_ADD;
    attachments->colorBlendOp = VK_BLEND_OP_ADD;
    attachments->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachments->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachments->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachments->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.attachmentCount = static_cast<uint32_t> ( COLOR_RENDER_TARGET_COUNT );
    info.pAttachments = attachments;
    info.logicOpEnable = VK_FALSE;
    info.logicOp = VK_LOGIC_OP_NO_OP;
    std::memset ( info.blendConstants, 0, sizeof ( info.blendConstants ) );

    return &info;
}

VkPipelineDepthStencilStateCreateInfo const* TexturePresentProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const noexcept
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.depthTestEnable = VK_FALSE;
    info.depthWriteEnable = VK_FALSE;
    info.depthBoundsTestEnable = VK_FALSE;
    info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    info.minDepthBounds = 0.0F;
    info.maxDepthBounds = 1.0F;
    info.stencilTestEnable = VK_FALSE;
    info.front.compareOp = VK_COMPARE_OP_ALWAYS;
    info.front.reference = UINT32_MAX;
    info.front.compareMask = UINT32_MAX;
    info.front.writeMask = 0x00U;
    info.front.failOp = VK_STENCIL_OP_KEEP;
    info.front.passOp = VK_STENCIL_OP_KEEP;
    info.front.depthFailOp = VK_STENCIL_OP_KEEP;
    std::memcpy ( &info.back, &info.front, sizeof ( info.back ) );

    return &info;
}

VkPipelineInputAssemblyStateCreateInfo const* TexturePresentProgram::InitInputAssemblyInfo (
    VkPipelineInputAssemblyStateCreateInfo &info
) const noexcept
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.primitiveRestartEnable = VK_FALSE;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    return &info;
}

bool TexturePresentProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_descriptorSetLayout.Init ( device ) )
        return false;

    VkDescriptorSetLayout layouts[] =
    {
        _descriptorSetLayout.GetLayout ()
    };

    constexpr static VkPushConstantRange const pushConstantRanges[]
    {
        {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0U,
            .size = static_cast<uint32_t> ( sizeof ( PushConstants ) )
        }
    };

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = static_cast<uint32_t> ( std::size ( layouts ) ),
        .pSetLayouts = layouts,
        .pushConstantRangeCount = static_cast<uint32_t> ( std::size ( pushConstantRanges ) ),
        .pPushConstantRanges = pushConstantRanges
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::TexturePresentProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "pbr::TexturePresentProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* TexturePresentProgram::InitMultisampleInfo (
    VkPipelineMultisampleStateCreateInfo &info
) const noexcept
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    info.sampleShadingEnable = VK_FALSE;
    info.pSampleMask = nullptr;
    info.minSampleShading = 0.0F;

    return &info;
}

VkPipelineRasterizationStateCreateInfo const* TexturePresentProgram::InitRasterizationInfo (
    VkPipelineRasterizationStateCreateInfo &info
) const noexcept
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_NONE;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.depthBiasEnable = VK_FALSE;
    info.depthClampEnable = VK_FALSE;
    info.lineWidth = 1.0F;
    info.depthBiasClamp = 0.0F;
    info.depthBiasConstantFactor = 0.0F;
    info.depthBiasSlopeFactor = 0.0F;

    return &info;
}

bool TexturePresentProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
) noexcept
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::TexturePresentProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "pbr::TexturePresentProgram::_vertexShader" )

    result = renderer.CreateShader ( _fragmentShader,
        FRAGMENT_SHADER,
        "Can't create fragment shader (pbr::TexturePresentProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "pbr::TexturePresentProgram::_fragmentShader" )

    VkPipelineShaderStageCreateInfo& vertexStage = sourceInfo[ 0U ];
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pNext = nullptr;
    vertexStage.flags = 0U;
    vertexStage.pSpecializationInfo = nullptr;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = _vertexShader;
    vertexStage.pName = VERTEX_SHADER_ENTRY_POINT;

    VkPipelineShaderStageCreateInfo& fragmentStage = sourceInfo[ 1U ];
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.pNext = nullptr;
    fragmentStage.flags = 0U;
    fragmentStage.pSpecializationInfo = nullptr;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = _fragmentShader;
    fragmentStage.pName = FRAGMENT_SHADER_ENTRY_POINT;

    targetInfo = sourceInfo;
    return true;
}

void TexturePresentProgram::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _fragmentShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, _fragmentShader, nullptr );
        _fragmentShader = VK_NULL_HANDLE;
        AV_UNREGISTER_SHADER_MODULE ( "pbr::TexturePresentProgram::_fragmentShader" )
    }

    if ( _vertexShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShader, nullptr );
    _vertexShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "pbr::TexturePresentProgram::_vertexShader" )
}

VkPipelineViewportStateCreateInfo const* TexturePresentProgram::InitViewportInfo (
    VkPipelineViewportStateCreateInfo &info,
    VkRect2D &scissorInfo,
    VkViewport &viewportInfo,
    VkExtent2D const &viewport
) const noexcept
{
    viewportInfo.x = 0.0F;
    viewportInfo.y = 0.0F;
    viewportInfo.minDepth = 0.0F;
    viewportInfo.maxDepth = 1.0F;
    viewportInfo.width = static_cast<float> ( viewport.width );
    viewportInfo.height = static_cast<float> ( viewport.height );

    scissorInfo.offset.x = 0;
    scissorInfo.offset.y = 0;
    scissorInfo.extent = viewport;

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.viewportCount = 1U;
    info.pViewports = &viewportInfo;
    info.scissorCount = 1U;
    info.pScissors = &scissorInfo;

    return &info;
}

VkPipelineVertexInputStateCreateInfo const* TexturePresentProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const noexcept
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.vertexBindingDescriptionCount = 0U;
    info.pVertexBindingDescriptions = binds;
    info.vertexAttributeDescriptionCount = 0U;
    info.pVertexAttributeDescriptions = attributes;

    return &info;
}

} // namespace pbr
