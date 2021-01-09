#include <pbr/point_light_shadowmap_generator_program.h>
#include <vertex_info.h>


namespace pbr {

constexpr static const char* VERTEX_SHADER = "shaders/point-light-shadowmap-generator-vs.spv";

constexpr static const uint32_t COLOR_RENDER_TARGET_COUNT = 0U;
constexpr static const size_t STAGE_COUNT = 1U;
constexpr static const size_t VERTEX_ATTRIBUTE_COUNT = 1U;

PointLightShadowmapGeneratorProgram::PointLightShadowmapGeneratorProgram () noexcept:
    Program ( "PointLightShadowmapGeneratorProgram" )
{
    // NOTHING
}

bool PointLightShadowmapGeneratorProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
)
{
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    VkVertexInputAttributeDescription attributeDescriptions;
    VkVertexInputBindingDescription bindingDescription;
    VkPipelineColorBlendStateCreateInfo blendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkRect2D scissorDescription;
    VkPipelineShaderStageCreateInfo stageInfo;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkViewport viewportDescription;
    VkPipelineViewportStateCreateInfo viewportInfo;

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.stageCount = static_cast<uint32_t> ( STAGE_COUNT );

    VkDevice device = renderer.GetDevice ();

    if ( !InitShaderInfo ( renderer, pipelineInfo.pStages, &stageInfo ) )
    {
        Destroy ( device );
        return false;
    }

    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo,
        &attributeDescriptions,
        &bindingDescription
    );

    pipelineInfo.pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo );
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = InitViewportInfo ( viewportInfo, scissorDescription, viewportDescription, viewport );
    pipelineInfo.pRasterizationState = InitRasterizationInfo ( rasterizationInfo );
    pipelineInfo.pMultisampleState = InitMultisampleInfo ( multisampleInfo );
    pipelineInfo.pDepthStencilState = InitDepthStencilInfo ( depthStencilInfo );
    pipelineInfo.pColorBlendState = InitColorBlendInfo ( blendInfo, nullptr );
    pipelineInfo.pDynamicState = nullptr;

    if ( !InitLayout ( renderer, pipelineInfo.layout ) )
    {
        Destroy ( device );
        return false;
    }

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = 0;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "PointLightShadowmapGeneratorProgram::Init",
        "Can't create pipeline"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_PIPELINE ( "PointLightShadowmapGeneratorProgram::_pipeline" )
    return true;
}

void PointLightShadowmapGeneratorProgram::Destroy ( VkDevice device )
{
    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "PointLightShadowmapGeneratorProgram::_pipelineLayout" )
    }

    _instanceLayout.Destroy ( device );

    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "PointLightShadowmapGeneratorProgram::_pipeline" )
    }

    if ( _vertexShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShader, nullptr );
    _vertexShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "PointLightShadowmapGeneratorProgram::_vertexShader" )
}

std::vector<DescriptorSetInfo> const& PointLightShadowmapGeneratorProgram::GetResourceInfo () const
{
    static std::vector<DescriptorSetInfo> const info
    {
        DescriptorSetInfo
        {
            ProgramResource ( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1U )
        }
    };

    return info;
}

void PointLightShadowmapGeneratorProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer,
    VkDescriptorSet sets
) const
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &sets,
        0U,
        nullptr
    );
}

VkPipelineColorBlendStateCreateInfo const* PointLightShadowmapGeneratorProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.logicOpEnable = VK_FALSE;
    info.logicOp = VK_LOGIC_OP_NO_OP;
    info.attachmentCount = COLOR_RENDER_TARGET_COUNT;
    info.pAttachments = attachments;
    memset ( info.blendConstants, 0, sizeof ( info.blendConstants ) );

    return &info;
}

VkPipelineDepthStencilStateCreateInfo const* PointLightShadowmapGeneratorProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = VK_TRUE;
    info.depthCompareOp = VK_COMPARE_OP_LESS;
    info.depthBoundsTestEnable = VK_FALSE;
    info.stencilTestEnable = VK_FALSE;

    info.front.failOp = VK_STENCIL_OP_KEEP;
    info.front.passOp = VK_STENCIL_OP_KEEP;
    info.front.depthFailOp = VK_STENCIL_OP_KEEP;
    info.front.compareOp = VK_COMPARE_OP_ALWAYS;
    info.front.compareMask = UINT32_MAX;
    info.front.writeMask = 0x00U;
    info.front.reference = UINT32_MAX;
    memcpy ( &info.back, &info.front, sizeof ( info.back ) );

    info.minDepthBounds = 0.0F;
    info.maxDepthBounds = 1.0F;

    return &info;
}

VkPipelineInputAssemblyStateCreateInfo const* PointLightShadowmapGeneratorProgram::InitInputAssemblyInfo (
    VkPipelineInputAssemblyStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    info.primitiveRestartEnable = VK_FALSE;

    return &info;
}

bool PointLightShadowmapGeneratorProgram::InitLayout ( android_vulkan::Renderer &renderer, VkPipelineLayout &layout )
{
    if ( !_instanceLayout.Init ( renderer ) )
        return false;

    VkDescriptorSetLayout layouts[] = { _instanceLayout.GetLayout () };

    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0U;
    layoutInfo.setLayoutCount = static_cast<uint32_t> ( std::size ( layouts ) );
    layoutInfo.pSetLayouts = layouts;
    layoutInfo.pushConstantRangeCount = 0U;
    layoutInfo.pPushConstantRanges = nullptr;

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( renderer.GetDevice (), &layoutInfo, nullptr, &_pipelineLayout ),
        "PointLightShadowmapGeneratorProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "PointLightShadowmapGeneratorProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* PointLightShadowmapGeneratorProgram::InitMultisampleInfo (
    VkPipelineMultisampleStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.sampleShadingEnable = VK_FALSE;
    info.minSampleShading = 0.0F;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;

    return &info;
}

VkPipelineRasterizationStateCreateInfo const* PointLightShadowmapGeneratorProgram::InitRasterizationInfo (
    VkPipelineRasterizationStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_BACK_BIT;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0F;
    info.depthBiasClamp = 0.0F;
    info.depthBiasSlopeFactor = 0.0F;
    info.lineWidth = 1.0F;

    return &info;
}

bool PointLightShadowmapGeneratorProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
)
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::PointLightShadowmapGeneratorProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "PointLightShadowmapGeneratorProgram::_vertexShader" )

    sourceInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    sourceInfo->pNext = nullptr;
    sourceInfo->flags = 0U;
    sourceInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
    sourceInfo->module = _vertexShader;
    sourceInfo->pName = VERTEX_SHADER_ENTRY_POINT;
    sourceInfo->pSpecializationInfo = nullptr;

    targetInfo = sourceInfo;
    return true;
}

VkPipelineViewportStateCreateInfo const* PointLightShadowmapGeneratorProgram::InitViewportInfo (
    VkPipelineViewportStateCreateInfo &info,
    VkRect2D &scissorInfo,
    VkViewport &viewportInfo,
    VkExtent2D const &viewport
) const
{
    viewportInfo.x = 0.0F;
    viewportInfo.y = 0.0F;
    viewportInfo.width = static_cast<float> ( viewport.width );
    viewportInfo.height = static_cast<float> ( viewport.height );
    viewportInfo.minDepth = 0.0F;
    viewportInfo.maxDepth = 1.0F;

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

VkPipelineVertexInputStateCreateInfo const* PointLightShadowmapGeneratorProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const
{
    binds->binding = 0U;
    binds->stride = static_cast<uint32_t> ( sizeof ( android_vulkan::VertexInfo ) );
    binds->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributes->location = 0U;
    attributes->binding = 0U;
    attributes->format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes->offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _vertex ) );

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
