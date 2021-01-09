#include <pbr/opaque_program.h>
#include <vertex_info.h>


namespace pbr {

constexpr static const char* VERTEX_SHADER = "shaders/common-opaque-vs.spv";
constexpr static const char* FRAGMENT_SHADER = "shaders/opaque-ps.spv";

constexpr static const size_t COLOR_RENDER_TARGET_COUNT = 4U;
constexpr static const size_t STAGE_COUNT = 2U;
constexpr static const size_t VERTEX_ATTRIBUTE_COUNT = 5U;

//----------------------------------------------------------------------------------------------------------------------

OpaqueProgram::OpaqueProgram () noexcept:
    Program ( "OpaqueProgram" ),
    _instanceLayout {},
    _textureLayout {}
{
    // NOTHING
}

bool OpaqueProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
)
{
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
    pipelineInfo.stageCount = static_cast<uint32_t> ( std::size ( stageInfo ) );

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
    pipelineInfo.pColorBlendState = InitColorBlendInfo ( blendInfo, attachmentInfo );
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
        "OpaqueProgram::Init",
        "Can't create pipeline"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_PIPELINE ( "OpaqueProgram::_pipeline" )
    return true;
}

void OpaqueProgram::Destroy ( VkDevice device )
{
    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "OpaqueProgram::_pipelineLayout" )
    }

    _textureLayout.Destroy ( device );
    _instanceLayout.Destroy ( device );

    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "OpaqueProgram::_pipeline" )
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
    static std::vector<DescriptorSetInfo> const info
    {
        DescriptorSetInfo
        {
            ProgramResource ( VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 5U ),
            ProgramResource ( VK_DESCRIPTOR_TYPE_SAMPLER, 5U )
        },

        DescriptorSetInfo
        {
            ProgramResource ( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1U )
        }
    };

    return info;
}

void OpaqueProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer,
    VkDescriptorSet const* sets,
    uint32_t startIndex,
    uint32_t count
) const
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        startIndex,
        count,
        sets,
        0U,
        nullptr
    );
}

VkPipelineColorBlendStateCreateInfo const* OpaqueProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const
{
    VkPipelineColorBlendAttachmentState& albedo = attachments[ 0U ];
    albedo.blendEnable = VK_FALSE;
    albedo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    albedo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    albedo.colorBlendOp = VK_BLEND_OP_ADD;
    albedo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    albedo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    albedo.alphaBlendOp = VK_BLEND_OP_ADD;

    albedo.colorWriteMask =
        AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT );

    constexpr auto const limit = static_cast<ptrdiff_t const> ( COLOR_RENDER_TARGET_COUNT );

    for ( ptrdiff_t i = 1; i < limit; ++i )
        memcpy ( attachments + i, &albedo, sizeof ( albedo ) );

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.logicOpEnable = VK_FALSE;
    info.logicOp = VK_LOGIC_OP_NO_OP;
    info.attachmentCount = static_cast<uint32_t> ( COLOR_RENDER_TARGET_COUNT );
    info.pAttachments = attachments;
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

VkPipelineInputAssemblyStateCreateInfo const* OpaqueProgram::InitInputAssemblyInfo (
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

bool OpaqueProgram::InitLayout ( android_vulkan::Renderer &renderer, VkPipelineLayout &layout )
{
    if ( !_instanceLayout.Init ( renderer ) )
        return false;

    if ( !_textureLayout.Init ( renderer ) )
        return false;

    VkDescriptorSetLayout const layouts[] =
    {
        _textureLayout.GetLayout (),
        _instanceLayout.GetLayout ()
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
        "OpaqueProgram::InitLayout",
        "Can't create pipeline layout"
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
    info.sampleShadingEnable = VK_FALSE;
    info.minSampleShading = 0.0F;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;

    return &info;
}

VkPipelineRasterizationStateCreateInfo const* OpaqueProgram::InitRasterizationInfo (
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

bool OpaqueProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
)
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
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = _vertexShader;
    vertexStage.pName = VERTEX_SHADER_ENTRY_POINT;
    vertexStage.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo& fragmentStage = sourceInfo[ 1U ];
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.pNext = nullptr;
    fragmentStage.flags = 0U;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = _fragmentShader;
    fragmentStage.pName = FRAGMENT_SHADER_ENTRY_POINT;
    fragmentStage.pSpecializationInfo = nullptr;

    targetInfo = sourceInfo;
    return true;
}

VkPipelineViewportStateCreateInfo const* OpaqueProgram::InitViewportInfo (
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

VkPipelineVertexInputStateCreateInfo const* OpaqueProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const
{
    binds->binding = 0U;
    binds->stride = static_cast<uint32_t> ( sizeof ( android_vulkan::VertexInfo ) );
    binds->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription& vertex = attributes[ 0U ];
    vertex.location = 0U;
    vertex.binding = 0U;
    vertex.format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _vertex ) );

    VkVertexInputAttributeDescription& uv = attributes[ 1U ];
    uv.location = 1U;
    uv.binding = 0U;
    uv.format = VK_FORMAT_R32G32_SFLOAT;
    uv.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _uv ) );

    VkVertexInputAttributeDescription& normal = attributes[ 2U ];
    normal.location = 2U;
    normal.binding = 0U;
    normal.format = VK_FORMAT_R32G32B32_SFLOAT;
    normal.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _normal ) );

    VkVertexInputAttributeDescription& tangent = attributes[ 3U ];
    tangent.location = 3U;
    tangent.binding = 0U;
    tangent.format = VK_FORMAT_R32G32B32_SFLOAT;
    tangent.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _tangent ) );

    VkVertexInputAttributeDescription& bitangent = attributes[ 4U ];
    bitangent.location = 4U;
    bitangent.binding = 0U;
    bitangent.format = VK_FORMAT_R32G32B32_SFLOAT;
    bitangent.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _bitangent ) );

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
