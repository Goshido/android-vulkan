#include <pbr/light_volume_program.h>


namespace pbr {

constexpr static const uint32_t COLOR_RENDER_TARGET_COUNT = 0U;
constexpr static const size_t STAGE_COUNT = 1U;
constexpr static const size_t VERTEX_ATTRIBUTE_COUNT = 1U;
constexpr static const char* VERTEX_SHADER = "shaders/light-volume-vs.spv";

//----------------------------------------------------------------------------------------------------------------------

LightVolumeProgram::LightVolumeProgram () noexcept:
    Program ( "pbr::LightVolumeProgram" ),
    _commonLayout {},
    _lightVolumeLayout {}
{
    // NOTHING
}

bool LightVolumeProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
)
{
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
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
    pipelineInfo.stageCount = std::size ( stageInfo );

    VkDevice device = renderer.GetDevice ();

    if ( !InitShaderInfo ( renderer, pipelineInfo.pStages, stageInfo ) )
    {
        Destroy ( device );
        return false;
    }

    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo,
        attributeDescriptions,
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
        "LightVolumeProgram::Init",
        "Can't create pipeline"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_PIPELINE ( "LightVolumeProgram::_pipeline" )
    return true;
}

void LightVolumeProgram::Destroy ( VkDevice device )
{
    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "LightVolumeProgram::_pipeline" )
    }

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "LightVolumeProgram::_pipelineLayout" )
    }

    _commonLayout.Destroy ( device );
    _lightVolumeLayout.Destroy ( device );

    if ( _vertexShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShader, nullptr );
    _vertexShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "LightVolumeProgram::_vertexShader" )
}

void LightVolumeProgram::SetTransform ( VkCommandBuffer commandBuffer, VkDescriptorSet transform )
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        1U,
        1U,
        &transform,
        0U,
        nullptr
    );
}

Program::DescriptorSetInfo const& LightVolumeProgram::GetResourceInfo () const
{
    static DescriptorSetInfo const null;
    return null;
}

VkPipelineColorBlendStateCreateInfo const* LightVolumeProgram::InitColorBlendInfo (
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

VkPipelineDepthStencilStateCreateInfo const* LightVolumeProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = VK_FALSE;
    info.depthCompareOp = VK_COMPARE_OP_GREATER;
    info.depthBoundsTestEnable = VK_FALSE;
    info.stencilTestEnable = VK_TRUE;

    info.front =
    {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .compareMask = UINT32_MAX,
        .writeMask = UINT32_MAX,
        .reference = 0U
    };

    info.back =
    {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .compareMask = UINT32_MAX,
        .writeMask = UINT32_MAX,
        .reference = 0U
    };

    info.minDepthBounds = 0.0F;
    info.maxDepthBounds = 1.0F;

    return &info;
}

VkPipelineInputAssemblyStateCreateInfo const* LightVolumeProgram::InitInputAssemblyInfo (
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

bool LightVolumeProgram::InitLayout ( android_vulkan::Renderer &renderer, VkPipelineLayout &layout )
{
    if ( !_commonLayout.Init ( renderer ) )
        return false;

    if ( !_lightVolumeLayout.Init ( renderer ) )
        return false;

    VkDescriptorSetLayout const layouts[] =
    {
        _commonLayout.GetLayout (),
        _lightVolumeLayout.GetLayout ()
    };

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = static_cast<uint32_t> ( std::size ( layouts ) ),
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( renderer.GetDevice (), &layoutInfo, nullptr, &_pipelineLayout ),
        "LightVolumeProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "LightVolumeProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* LightVolumeProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* LightVolumeProgram::InitRasterizationInfo (
    VkPipelineRasterizationStateCreateInfo &info
) const
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_NONE;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0F;
    info.depthBiasClamp = 0.0F;
    info.depthBiasSlopeFactor = 0.0F;
    info.lineWidth = 1.0F;

    return &info;
}

bool LightVolumeProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
)
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::LightVolumeProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "LightVolumeProgram::_vertexShader" )

    VkPipelineShaderStageCreateInfo& vertexStage = sourceInfo[ 0U ];
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pNext = nullptr;
    vertexStage.flags = 0U;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = _vertexShader;
    vertexStage.pName = VERTEX_SHADER_ENTRY_POINT;
    vertexStage.pSpecializationInfo = nullptr;

    targetInfo = sourceInfo;
    return true;
}

VkPipelineViewportStateCreateInfo const* LightVolumeProgram::InitViewportInfo (
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

VkPipelineVertexInputStateCreateInfo const* LightVolumeProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const
{
    binds->binding = 0U;
    binds->stride = static_cast<uint32_t> ( sizeof ( GXVec3 ) );
    binds->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributes->location = 0U;
    attributes->binding = 0U;
    attributes->format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes->offset = 0U;

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
