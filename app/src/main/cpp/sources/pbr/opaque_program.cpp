#include <pbr/opaque_program.h>
#include <vertex_info.h>
#include <vulkan_utils.h>


namespace pbr {

constexpr static const char* VERTEX_SHADER = "shaders/common-opaque-vs.spv";
constexpr static const char* VERTEX_SHADER_ENTRY_POINT = "VS";

constexpr static const char* FRAGMENT_SHADER = "shaders/opaque-ps.spv";
constexpr static const char* FRAGMENT_SHADER_ENTRY_POINT = "PS";

constexpr static const size_t STAGE_COUNT = 2U;

OpaqueProgram::OpaqueProgram ():
    Program ( "pbr::OpaqueProgram" ),
    _albedoTexture ( nullptr ),
    _albedoSampler ( VK_NULL_HANDLE ),
    _emissionTexture ( nullptr ),
    _emissionSampler ( VK_NULL_HANDLE ),
    _normalTexture ( nullptr ),
    _normalSampler ( VK_NULL_HANDLE ),
    _paramTexture ( nullptr ),
    _paramSampler ( VK_NULL_HANDLE ),
    _descriptorSetLayouts { VK_NULL_HANDLE, VK_NULL_HANDLE }
{
    // NOTHING
}

bool OpaqueProgram::Init ( android_vulkan::Renderer &renderer, VkRenderPass renderPass, const VkExtent2D &viewport )
{
    assert ( _state == eProgramState::Unknown );
    _state = eProgramState::Initializing;

    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::OpaqueProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "OpaqueProgram::_vertexShader" )

    result = renderer.CreateShader ( _fragmentShader,
        FRAGMENT_SHADER,
        "Can't create fragment shader (pbr::OpaqueProgram)"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_SHADER_MODULE ( "OpaqueProgram::_fragmentShader" )

    VkDevice device = renderer.GetDevice ();

    VkDescriptorSetLayoutBinding firstBindingInfo;
    firstBindingInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    firstBindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    firstBindingInfo.descriptorCount = 1U;
    firstBindingInfo.binding = 0U;
    firstBindingInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0U;
    descriptorSetLayoutInfo.bindingCount = 1U;
    descriptorSetLayoutInfo.pBindings = &firstBindingInfo;

    result = renderer.CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, _descriptorSetLayouts ),
        "OpaqueProgram::Init",
        "Can't create first descriptor set layout (pbr::OpaqueProgram)"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueProgram::_descriptorSetLayouts[ 0U ]" )

    VkDescriptorSetLayoutBinding secondBindingInfo[ 8U ];
    VkDescriptorSetLayoutBinding& diffuseTextureBind = secondBindingInfo[ 0U ];
    diffuseTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    diffuseTextureBind.descriptorCount = 1U;
    diffuseTextureBind.binding = 0U;
    diffuseTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& diffuseSamplerBind = secondBindingInfo[ 1U ];
    diffuseSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    diffuseSamplerBind.descriptorCount = 1U;
    diffuseSamplerBind.binding = 1U;
    diffuseSamplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& emissionTextureBind = secondBindingInfo[ 2U ];
    emissionTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    emissionTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    emissionTextureBind.descriptorCount = 1U;
    emissionTextureBind.binding = 2U;
    emissionTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& emissionSamplerBind = secondBindingInfo[ 3U ];
    emissionSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    emissionSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    emissionSamplerBind.descriptorCount = 1U;
    emissionSamplerBind.binding = 3U;
    emissionSamplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalTextureBind = secondBindingInfo[ 4U ];
    normalTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    normalTextureBind.descriptorCount = 1U;
    normalTextureBind.binding = 4U;
    normalTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalSamplerBind = secondBindingInfo[ 5U ];
    normalSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    normalSamplerBind.descriptorCount = 1U;
    normalSamplerBind.binding = 5U;
    normalSamplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& paramTextureBind = secondBindingInfo[ 6U ];
    paramTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    paramTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    paramTextureBind.descriptorCount = 1U;
    paramTextureBind.binding = 6U;
    paramTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& paramSamplerBind = secondBindingInfo[ 7U ];
    paramSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    paramSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    paramSamplerBind.descriptorCount = 1U;
    paramSamplerBind.binding = 7U;
    paramSamplerBind.pImmutableSamplers = nullptr;

    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t> ( std::size ( secondBindingInfo ) );
    descriptorSetLayoutInfo.pBindings = secondBindingInfo;

    result = renderer.CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, _descriptorSetLayouts + 1U ),
        "OpaqueProgram::Init",
        "Can't create second descriptor set layout (pbr::OpaqueProgram)"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueProgram::_descriptorSetLayouts[ 1U ]" )

    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0U;
    pushConstantRange.size = static_cast<uint32_t> ( sizeof ( GXMat4 ) );

    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0U;
    layoutInfo.pushConstantRangeCount = 1U;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    layoutInfo.setLayoutCount = static_cast<uint32_t> ( std::size ( _descriptorSetLayouts ) );
    layoutInfo.pSetLayouts = _descriptorSetLayouts;

    result = renderer.CheckVkResult ( vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "OpaqueProgram::Init",
        "Can't create pipeline layout (pbr::OpaqueProgram)"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_PIPELINE_LAYOUT ( "OpaqueProgram::_pipelineLayout" )

    VkPipelineShaderStageCreateInfo stageInfo[ STAGE_COUNT ];
    VkPipelineShaderStageCreateInfo& vertexStage = stageInfo[ 0U ];
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pNext = nullptr;
    vertexStage.flags = 0U;
    vertexStage.pSpecializationInfo = nullptr;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = _vertexShader;
    vertexStage.pName = VERTEX_SHADER_ENTRY_POINT;

    VkPipelineShaderStageCreateInfo& fragmentStage = stageInfo[ 1U ];
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.pNext = nullptr;
    fragmentStage.flags = 0U;
    fragmentStage.pSpecializationInfo = nullptr;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = _fragmentShader;
    fragmentStage.pName = FRAGMENT_SHADER_ENTRY_POINT;

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyInfo.pNext = nullptr;
    assemblyInfo.flags = 0U;
    assemblyInfo.primitiveRestartEnable = VK_FALSE;
    assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0U;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDescription.stride = static_cast<uint32_t> ( sizeof ( android_vulkan::VertexInfo ) );

    VkVertexInputAttributeDescription attributeDescriptions[ 5U ];
    VkVertexInputAttributeDescription& vertexDescription = attributeDescriptions[ 0U ];
    vertexDescription.binding = 0U;
    vertexDescription.location = 0U;
    vertexDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _vertex ) );
    vertexDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& uvDescription = attributeDescriptions[ 1U ];
    uvDescription.binding = 0U;
    uvDescription.location = 1U;
    uvDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _uv ) );
    uvDescription.format = VK_FORMAT_R32G32_SFLOAT;

    VkVertexInputAttributeDescription& normalDescription = attributeDescriptions[ 2U ];
    normalDescription.binding = 0U;
    normalDescription.location = 2U;
    normalDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _normal ) );
    normalDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& tangentDescription = attributeDescriptions[ 3U ];
    tangentDescription.binding = 0U;
    tangentDescription.location = 3U;
    tangentDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _tangent ) );
    tangentDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& bitangentDescription = attributeDescriptions[ 4U ];
    bitangentDescription.binding = 0U;
    bitangentDescription.location = 4U;
    bitangentDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _bitangent ) );
    bitangentDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0U;
    vertexInputInfo.vertexBindingDescriptionCount = 1U;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t> ( std::size ( attributeDescriptions ) );
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.pNext = nullptr;
    depthStencilInfo.flags = 0U;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.minDepthBounds = 0.0F;
    depthStencilInfo.maxDepthBounds = 1.0F;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    depthStencilInfo.front.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilInfo.front.reference = UINT32_MAX;
    depthStencilInfo.front.compareMask = UINT32_MAX;
    depthStencilInfo.front.writeMask = 0x00U;
    depthStencilInfo.front.failOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.front.passOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.front.depthFailOp = VK_STENCIL_OP_KEEP;
    memcpy ( &depthStencilInfo.back, &depthStencilInfo.front, sizeof ( depthStencilInfo.back ) );

    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.pNext = nullptr;
    rasterizationInfo.flags = 0U;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.lineWidth = 1.0F;
    rasterizationInfo.depthBiasClamp = 0.0F;
    rasterizationInfo.depthBiasConstantFactor = 0.0F;
    rasterizationInfo.depthBiasSlopeFactor = 0.0F;

    VkViewport viewportDescription;
    viewportDescription.x = 0.0F;
    viewportDescription.y = 0.0F;
    viewportDescription.minDepth = 0.0F;
    viewportDescription.maxDepth = 1.0F;
    viewportDescription.width = static_cast<float> ( viewport.width );
    viewportDescription.height = static_cast<float> ( viewport.height );

    VkRect2D scissorDescription;
    scissorDescription.offset.x = 0;
    scissorDescription.offset.y = 0;
    scissorDescription.extent = viewport;

    VkPipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.pNext = nullptr;
    viewportInfo.flags = 0U;
    viewportInfo.viewportCount = 1U;
    viewportInfo.pViewports = &viewportDescription;
    viewportInfo.scissorCount = 1U;
    viewportInfo.pScissors = &scissorDescription;

    VkPipelineColorBlendAttachmentState attachmentInfo[ 4U ];
    VkPipelineColorBlendAttachmentState& albedoDescription = attachmentInfo[ 0U ];
    albedoDescription.blendEnable = VK_FALSE;

    albedoDescription.colorWriteMask =
        AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT );

    albedoDescription.alphaBlendOp = VK_BLEND_OP_ADD;
    albedoDescription.colorBlendOp = VK_BLEND_OP_ADD;
    albedoDescription.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    albedoDescription.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    albedoDescription.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    albedoDescription.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    constexpr const auto limit = static_cast<const ptrdiff_t> ( std::size ( attachmentInfo ) );

    for ( ptrdiff_t i = 1; i < limit; ++i )
        memcpy ( attachmentInfo + i, &albedoDescription, sizeof ( albedoDescription ) );

    VkPipelineColorBlendStateCreateInfo blendInfo;
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.pNext = nullptr;
    blendInfo.flags = 0U;
    blendInfo.attachmentCount = static_cast<uint32_t> ( std::size ( attachmentInfo ) );
    blendInfo.pAttachments = attachmentInfo;
    blendInfo.logicOpEnable = VK_FALSE;
    blendInfo.logicOp = VK_LOGIC_OP_NO_OP;
    memset ( blendInfo.blendConstants, 0, sizeof ( blendInfo.blendConstants ) );

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.pNext = nullptr;
    multisampleInfo.flags = 0U;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.pSampleMask = nullptr;
    multisampleInfo.minSampleShading = 0.0F;

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.subpass = 0U;
    pipelineInfo.stageCount = std::size ( stageInfo );
    pipelineInfo.pStages = stageInfo;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.basePipelineIndex = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pInputAssemblyState = &assemblyInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pColorBlendState = &blendInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;

    result = renderer.CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "OpaqueProgram::Init",
        "Can't create pipeline (pbr::OpaqueProgram)"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_PIPELINE ( "OpaqueProgram::_pipeline" )
    _state = eProgramState::Ready;

    return true;
}

void OpaqueProgram::Destroy ( android_vulkan::Renderer &renderer )
{
    assert ( _state != eProgramState::Unknown );

    VkDevice device = renderer.GetDevice ();

    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "OpaqueProgram::_pipeline" )
    }

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "OpaqueProgram::_pipelineLayout" )
    }

    VkDescriptorSetLayout& secondLayout = _descriptorSetLayouts[ 1U ];

    if ( secondLayout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout ( device, secondLayout, nullptr );
        secondLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueProgram::_descriptorSetLayouts[ 1U ]" )
    }

    VkDescriptorSetLayout& firstLayout = _descriptorSetLayouts[ 0U ];

    if ( firstLayout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout ( device, firstLayout, nullptr );
        firstLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueProgram::_descriptorSetLayouts[ 0U ]" )
    }

    if ( _fragmentShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, _fragmentShader, nullptr );
        _fragmentShader = VK_NULL_HANDLE;
        AV_UNREGISTER_SHADER_MODULE ( "OpaqueProgram::_fragmentShader" )
    }

    if ( _vertexShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShader, nullptr );
    _vertexShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "OpaqueProgram::_vertexShader" )
}

void OpaqueProgram::SetAlbedo ( android_vulkan::Texture2D &texture, VkSampler sampler )
{
    assert ( _state == eProgramState::Setup );
    _albedoTexture = &texture;
    _albedoSampler = sampler;
}

void OpaqueProgram::SetEmission ( android_vulkan::Texture2D &texture, VkSampler sampler )
{
    assert ( _state == eProgramState::Setup );
    _emissionTexture = &texture;
    _emissionSampler = sampler;
}

void OpaqueProgram::SetNormal ( android_vulkan::Texture2D &texture, VkSampler sampler )
{
    assert ( _state == eProgramState::Setup );
    _normalTexture = &texture;
    _normalSampler = sampler;
}

void OpaqueProgram::SetParams ( android_vulkan::Texture2D &texture, VkSampler sampler )
{
    assert ( _state == eProgramState::Setup );
    _paramTexture = &texture;
    _paramSampler = sampler;
}

void OpaqueProgram::BeginSetup ()
{
    assert ( _state != eProgramState::Setup );
    _state = eProgramState::Setup;
}

void OpaqueProgram::EndSetup ()
{
    assert ( _state == eProgramState::Setup );
    _state = eProgramState::Ready;
}

bool OpaqueProgram::Bind ( android_vulkan::Renderer &/*renderer*/ )
{
    assert ( _state == eProgramState::Ready );
    _state = eProgramState::Bind;

    // TODO

    assert ( !"OpaqueProgram::Bind - Implement me!" );
    return false;
}

} // namespace pbr
