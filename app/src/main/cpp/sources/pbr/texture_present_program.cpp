#include <pbr/texture_present_program.h>


namespace pbr {

constexpr static const char* VERTEX_SHADER = "shaders/screen-quad-vs.spv";
constexpr static const char* FRAGMENT_SHADER = "shaders/texture-present-ps.spv";

constexpr static const size_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr static const size_t STAGE_COUNT = 2U;

//----------------------------------------------------------------------------------------------------------------------

TexturePresentProgram::TexturePresentProgram ():
    Program ( "pbr::TexturePresentProgram" )
{
    // NOTHING
}

bool TexturePresentProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    const VkExtent2D &viewport
)
{
    assert ( _state == eProgramState::Unknown );
    _state = eProgramState::Initializing;

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
    pipelineInfo.subpass = 0U;
    pipelineInfo.stageCount = std::size ( stageInfo );
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.basePipelineIndex = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo );
    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo, nullptr, nullptr );
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pDepthStencilState = InitDepthStencilInfo ( depthStencilInfo );
    pipelineInfo.pRasterizationState = InitRasterizationInfo ( rasterizationInfo );
    pipelineInfo.pViewportState = InitViewportInfo ( viewportInfo, scissorDescription, viewportDescription, viewport );
    pipelineInfo.pColorBlendState = InitColorBlendInfo ( blendInfo, attachmentInfo );
    pipelineInfo.pMultisampleState = InitMultisampleInfo ( multisampleInfo );

    if ( !InitShaderInfo ( pipelineInfo.pStages, stageInfo, renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !InitLayout ( pipelineInfo.layout, renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    const bool result = renderer.CheckVkResult (
        vkCreateGraphicsPipelines ( renderer.GetDevice (), VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "TexturePresentProgram::Init",
        "Can't create pipeline (pbr::TexturePresentProgram)"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_PIPELINE ( "TexturePresentProgram::_pipeline" )
    _state = eProgramState::Ready;

    return true;
}

void TexturePresentProgram::Destroy ( android_vulkan::Renderer &renderer )
{
    assert ( _state != eProgramState::Unknown );

    VkDevice device = renderer.GetDevice ();

    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "TexturePresentProgram::_pipeline" )
    }

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "TexturePresentProgram::_pipelineLayout" )
    }

    if ( _descriptorSetLayout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
        _descriptorSetLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "TexturePresentProgram::_descriptorSetLayout" )
    }

    if ( _fragmentShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, _fragmentShader, nullptr );
        _fragmentShader = VK_NULL_HANDLE;
        AV_UNREGISTER_SHADER_MODULE ( "TexturePresentProgram::_fragmentShader" )
    }

    if ( _vertexShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShader, nullptr );
    _vertexShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "TexturePresentProgram::_vertexShader" )
}

void TexturePresentProgram::BeginSetup ()
{
    assert ( !"TexturePresentProgram::BeginSetup - Implement me!" );
}

void TexturePresentProgram::EndSetup ()
{
    assert ( !"TexturePresentProgram::EndSetup - Implement me!" );
}

bool TexturePresentProgram::Bind ( android_vulkan::Renderer &/*renderer*/ )
{
    assert ( !"TexturePresentProgram::Bind - Implement me!" );
    return false;
}

const VkPipelineColorBlendStateCreateInfo* TexturePresentProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const
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
    memset ( info.blendConstants, 0, sizeof ( info.blendConstants ) );

    return &info;
}

const VkPipelineDepthStencilStateCreateInfo* TexturePresentProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const
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
    memcpy ( &info.back, &info.front, sizeof ( info.back ) );

    return &info;
}

const VkPipelineInputAssemblyStateCreateInfo* TexturePresentProgram::InitInputAssemblyInfo (
    VkPipelineInputAssemblyStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.primitiveRestartEnable = VK_FALSE;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    return &info;
}

bool TexturePresentProgram::InitLayout ( VkPipelineLayout &layout, android_vulkan::Renderer &renderer )
{
    VkDescriptorSetLayoutBinding bindingInfo[ 2U ];
    VkDescriptorSetLayoutBinding& textureBind = bindingInfo[ 0U ];
    textureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    textureBind.descriptorCount = 1U;
    textureBind.binding = 0U;
    textureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& samplerBind = bindingInfo[ 1U ];
    samplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerBind.descriptorCount = 1U;
    samplerBind.binding = 1U;
    samplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0U;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t> ( std::size ( bindingInfo ) );
    descriptorSetLayoutInfo.pBindings = bindingInfo;

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
        "TexturePresentProgram::InitLayout",
        "Can't create descriptor set layout (pbr::TexturePresentProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "TexturePresentProgram::_descriptorSetLayout" )

    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0U;
    pushConstantRange.size = static_cast<uint32_t> ( sizeof ( PushConstants ) );

    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0U;
    layoutInfo.pushConstantRangeCount = 1U;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    layoutInfo.setLayoutCount = 1U;
    layoutInfo.pSetLayouts = &_descriptorSetLayout;

    result = renderer.CheckVkResult ( vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "TexturePresentProgram::InitLayout",
        "Can't create pipeline layout (pbr::TexturePresentProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "TexturePresentProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

const VkPipelineMultisampleStateCreateInfo* TexturePresentProgram::InitMultisampleInfo (
    VkPipelineMultisampleStateCreateInfo &info
) const
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

const VkPipelineRasterizationStateCreateInfo* TexturePresentProgram::InitRasterizationInfo (
    VkPipelineRasterizationStateCreateInfo &info
) const
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

bool TexturePresentProgram::InitShaderInfo ( const VkPipelineShaderStageCreateInfo* &targetInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo,
    android_vulkan::Renderer &renderer
)
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::TexturePresentProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "TexturePresentProgram::_vertexShader" )

    result = renderer.CreateShader ( _fragmentShader,
        FRAGMENT_SHADER,
        "Can't create fragment shader (pbr::TexturePresentProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "TexturePresentProgram::_fragmentShader" )

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

const VkPipelineViewportStateCreateInfo* TexturePresentProgram::InitViewportInfo (
    VkPipelineViewportStateCreateInfo &info,
    VkRect2D &scissorInfo,
    VkViewport &viewportInfo,
    const VkExtent2D &viewport
) const
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

const VkPipelineVertexInputStateCreateInfo* TexturePresentProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const
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
