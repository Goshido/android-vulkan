#include <pbr/opaque_program.h>
#include <vertex_info.h>
#include <vulkan_utils.h>


namespace pbr {

constexpr static const char* VERTEX_SHADER = "shaders/common-opaque-vs.spv";
constexpr static const char* FRAGMENT_SHADER = "shaders/opaque-ps.spv";

constexpr static const size_t COLOR_RENDER_TARGET_COUNT = 4U;
constexpr static const size_t DESCRIPTOR_SET_COUNT = 1U;
constexpr static const size_t STAGE_COUNT = 2U;
constexpr static const size_t VERTEX_ATTRIBUTE_COUNT = 5U;

//----------------------------------------------------------------------------------------------------------------------

OpaqueProgram::OpaqueProgram ():
    Program ( "pbr::OpaqueProgram", DESCRIPTOR_SET_COUNT )
{
    // NOTHING
}

bool OpaqueProgram::Init ( android_vulkan::Renderer &renderer, VkRenderPass renderPass, const VkExtent2D &viewport )
{
    assert ( _state == eProgramState::Unknown );
    _state = eProgramState::Initializing;

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    VkPipelineColorBlendAttachmentState attachmentInfo[ COLOR_RENDER_TARGET_COUNT ];
    VkVertexInputAttributeDescription attributeDescriptions[ VERTEX_ATTRIBUTE_COUNT ];
    VkVertexInputBindingDescription bindingDescription;
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
    pipelineInfo.stageCount = static_cast<uint32_t> ( std::size ( stageInfo ) );
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.basePipelineIndex = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo );

    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo,
        attributeDescriptions,
        &bindingDescription
    );

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

    for ( auto& descriptorSet : _descriptorSetLayouts )
    {
        if ( descriptorSet == VK_NULL_HANDLE )
            continue;

        vkDestroyDescriptorSetLayout ( device, descriptorSet, nullptr );
        descriptorSet = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueProgram::_descriptorSetLayouts" )
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

std::vector<DescriptorSetInfo> const& OpaqueProgram::GetResourceInfo () const
{
    static const std::vector<DescriptorSetInfo> info
    {
        DescriptorSetInfo
        {
            ProgramResource ( VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4U ),
            ProgramResource ( VK_DESCRIPTOR_TYPE_SAMPLER, 4U )
        }
    };

    return info;
}

void OpaqueProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet ) const
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &descriptorSet,
        0U,
        nullptr
    );
}

void OpaqueProgram::SetTransform ( VkCommandBuffer commandBuffer, PushConstants const &transform ) const
{
    vkCmdPushConstants ( commandBuffer,
        _pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0U,
        sizeof ( PushConstants ),
        &transform
    );
}

VkPipelineColorBlendStateCreateInfo const* OpaqueProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const
{
    VkPipelineColorBlendAttachmentState& albedoDescription = attachments[ 0U ];
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

    constexpr const auto limit = static_cast<const ptrdiff_t> ( COLOR_RENDER_TARGET_COUNT );

    for ( ptrdiff_t i = 1; i < limit; ++i )
        memcpy ( attachments + i, &albedoDescription, sizeof ( albedoDescription ) );

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

VkPipelineDepthStencilStateCreateInfo const* OpaqueProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = VK_TRUE;
    info.depthBoundsTestEnable = VK_FALSE;
    info.depthCompareOp = VK_COMPARE_OP_LESS;
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

VkPipelineInputAssemblyStateCreateInfo const* OpaqueProgram::InitInputAssemblyInfo (
    VkPipelineInputAssemblyStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.primitiveRestartEnable = VK_FALSE;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    return &info;
}

bool OpaqueProgram::InitLayout ( VkPipelineLayout &layout, android_vulkan::Renderer &renderer )
{
    VkDescriptorSetLayoutBinding bindingInfo[ 8U ];
    VkDescriptorSetLayoutBinding& diffuseTextureBind = bindingInfo[ 0U ];
    diffuseTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    diffuseTextureBind.descriptorCount = 1U;
    diffuseTextureBind.binding = 0U;
    diffuseTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& diffuseSamplerBind = bindingInfo[ 1U ];
    diffuseSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    diffuseSamplerBind.descriptorCount = 1U;
    diffuseSamplerBind.binding = 1U;
    diffuseSamplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& emissionTextureBind = bindingInfo[ 2U ];
    emissionTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    emissionTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    emissionTextureBind.descriptorCount = 1U;
    emissionTextureBind.binding = 2U;
    emissionTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& emissionSamplerBind = bindingInfo[ 3U ];
    emissionSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    emissionSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    emissionSamplerBind.descriptorCount = 1U;
    emissionSamplerBind.binding = 3U;
    emissionSamplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalTextureBind = bindingInfo[ 4U ];
    normalTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    normalTextureBind.descriptorCount = 1U;
    normalTextureBind.binding = 4U;
    normalTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalSamplerBind = bindingInfo[ 5U ];
    normalSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    normalSamplerBind.descriptorCount = 1U;
    normalSamplerBind.binding = 5U;
    normalSamplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& paramTextureBind = bindingInfo[ 6U ];
    paramTextureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    paramTextureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    paramTextureBind.descriptorCount = 1U;
    paramTextureBind.binding = 6U;
    paramTextureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& paramSamplerBind = bindingInfo[ 7U ];
    paramSamplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    paramSamplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    paramSamplerBind.descriptorCount = 1U;
    paramSamplerBind.binding = 7U;
    paramSamplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0U;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t> ( std::size ( bindingInfo ) );
    descriptorSetLayoutInfo.pBindings = bindingInfo;

    VkDevice device = renderer.GetDevice ();
    VkDescriptorSetLayout& descriptorSetLayout = _descriptorSetLayouts[ 0U ];

    bool result = renderer.CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout ),
        "OpaqueProgram::InitLayout",
        "Can't create descriptor set layout (pbr::OpaqueProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueProgram::_descriptorSetLayouts" )

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
    layoutInfo.pSetLayouts = &descriptorSetLayout;

    result = renderer.CheckVkResult ( vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "OpaqueProgram::InitLayout",
        "Can't create pipeline layout (pbr::OpaqueProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "OpaqueProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* OpaqueProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* OpaqueProgram::InitRasterizationInfo (
    VkPipelineRasterizationStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_BACK_BIT;
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

bool OpaqueProgram::InitShaderInfo ( VkPipelineShaderStageCreateInfo const* &targetInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo,
    android_vulkan::Renderer &renderer )
{
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
        return false;

    AV_REGISTER_SHADER_MODULE ( "OpaqueProgram::_fragmentShader" )

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

VkPipelineViewportStateCreateInfo const* OpaqueProgram::InitViewportInfo ( VkPipelineViewportStateCreateInfo &info,
    VkRect2D &scissorInfo,
    VkViewport &viewportInfo,
    VkExtent2D const &viewport
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

VkPipelineVertexInputStateCreateInfo const* OpaqueProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const
{
    binds->binding = 0U;
    binds->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binds->stride = static_cast<uint32_t> ( sizeof ( android_vulkan::VertexInfo ) );

    VkVertexInputAttributeDescription& vertexDescription = attributes[ 0U ];
    vertexDescription.binding = 0U;
    vertexDescription.location = 0U;
    vertexDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _vertex ) );
    vertexDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& uvDescription = attributes[ 1U ];
    uvDescription.binding = 0U;
    uvDescription.location = 1U;
    uvDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _uv ) );
    uvDescription.format = VK_FORMAT_R32G32_SFLOAT;

    VkVertexInputAttributeDescription& normalDescription = attributes[ 2U ];
    normalDescription.binding = 0U;
    normalDescription.location = 2U;
    normalDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _normal ) );
    normalDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& tangentDescription = attributes[ 3U ];
    tangentDescription.binding = 0U;
    tangentDescription.location = 3U;
    tangentDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _tangent ) );
    tangentDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& bitangentDescription = attributes[ 4U ];
    bitangentDescription.binding = 0U;
    bitangentDescription.location = 4U;
    bitangentDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _bitangent ) );
    bitangentDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.vertexBindingDescriptionCount = 1U;
    info.pVertexBindingDescriptions = binds;
    info.vertexAttributeDescriptionCount = static_cast<uint32_t> ( VERTEX_ATTRIBUTE_COUNT );
    info.pVertexAttributeDescriptions = attributes;

    return &info;
}

} // namespace pbr
