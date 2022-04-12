#include <pbr/reflection_local_program.h>
#include <pbr/light_volume_program.h>


namespace pbr {

constexpr static size_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr static size_t STAGE_COUNT = 2U;
constexpr static size_t VERTEX_ATTRIBUTE_COUNT = 1U;

constexpr static const char* VERTEX_SHADER = "shaders/light-volume-vs.spv";
constexpr static const char* FRAGMENT_SHADER = "shaders/reflection-local-ps.spv";

//----------------------------------------------------------------------------------------------------------------------

ReflectionLocalProgram::ReflectionLocalProgram ():
    LightLightupBaseProgram ( "pbr::ReflectionLocalProgram" ),
    _commonLayout {},
    _lightVolumeLayout {},
    _reflectionLayout {}
{
    // NOTHING
}

bool ReflectionLocalProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
) noexcept
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
        "pbr::ReflectionLocalProgram::Init",
        "Can't create pipeline"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_PIPELINE ( "ReflectionLocalProgram::_pipeline" )
    DestroyShaderModules ( device );
    return true;
}

void ReflectionLocalProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE ( "ReflectionLocalProgram::_pipeline" )
    }

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "ReflectionLocalProgram::_pipelineLayout" )
    }

    _reflectionLayout.Destroy ( device );
    _lightVolumeLayout.Destroy ( device );
    _commonLayout.Destroy ( device );

    DestroyShaderModules ( device );
}

Program::DescriptorSetInfo const& ReflectionLocalProgram::GetResourceInfo () const noexcept
{
    static DescriptorSetInfo const info
    {
        {
            {
                .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                .descriptorCount = 4U
            },
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = 1U
            },
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = 2U
            },
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1U
            }
        },
        {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1U
            }
        },
        {
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = 1U
            },
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1U
            }
        }
    };

    return info;
}

void ReflectionLocalProgram::SetLightData ( VkCommandBuffer commandBuffer, VkDescriptorSet lightData ) const noexcept
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        2U,
        1U,
        &lightData,
        0U,
        nullptr
    );
}

VkPipelineColorBlendStateCreateInfo const* ReflectionLocalProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    VkPipelineColorBlendAttachmentState& hdrAccumulator = attachments[ 0U ];
    hdrAccumulator.blendEnable = VK_TRUE;
    hdrAccumulator.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    hdrAccumulator.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    hdrAccumulator.colorBlendOp = VK_BLEND_OP_ADD;
    hdrAccumulator.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    hdrAccumulator.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    hdrAccumulator.alphaBlendOp = VK_BLEND_OP_ADD;

    hdrAccumulator.colorWriteMask = AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT );

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

VkPipelineDepthStencilStateCreateInfo const* ReflectionLocalProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const noexcept
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
        .passOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .compareMask = UINT32_MAX,
        .writeMask = 0U,
        .reference = 0U
    };

    info.back =
    {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp = VK_COMPARE_OP_EQUAL,
        .compareMask = UINT32_MAX,
        .writeMask = 0U,
        .reference = LightVolumeProgram::GetLightVolumeStencilValue ()
    };

    info.minDepthBounds = 0.0F;
    info.maxDepthBounds = 1.0F;

    return &info;
}

VkPipelineInputAssemblyStateCreateInfo const* ReflectionLocalProgram::InitInputAssemblyInfo (
    VkPipelineInputAssemblyStateCreateInfo &info
) const noexcept
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    info.primitiveRestartEnable = VK_FALSE;

    return &info;
}

bool ReflectionLocalProgram::InitLayout ( android_vulkan::Renderer &renderer, VkPipelineLayout &layout ) noexcept
{
    if ( !_commonLayout.Init ( renderer ) )
        return false;

    if ( !_lightVolumeLayout.Init ( renderer ) )
        return false;

    if ( !_reflectionLayout.Init ( renderer ) )
        return false;

    VkDescriptorSetLayout const layouts[] =
    {
        _commonLayout.GetLayout (),
        _lightVolumeLayout.GetLayout (),
        _reflectionLayout.GetLayout ()
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
        "pbr::ReflectionLocalProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "ReflectionLocalProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* ReflectionLocalProgram::InitMultisampleInfo (
    VkPipelineMultisampleStateCreateInfo &info
) const noexcept
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

VkPipelineRasterizationStateCreateInfo const* ReflectionLocalProgram::InitRasterizationInfo (
    VkPipelineRasterizationStateCreateInfo &info
) const noexcept
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_FRONT_BIT;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0F;
    info.depthBiasClamp = 0.0F;
    info.depthBiasSlopeFactor = 0.0F;
    info.lineWidth = 1.0F;

    return &info;
}

bool ReflectionLocalProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
) noexcept
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::ReflectionLocalProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "ReflectionLocalProgram::_vertexShader" )

    result = renderer.CreateShader ( _fragmentShader,
        FRAGMENT_SHADER,
        "Can't create fragment shader (pbr::ReflectionLocalProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "ReflectionLocalProgram::_fragmentShader" )

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

void ReflectionLocalProgram::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _fragmentShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, _fragmentShader, nullptr );
        _fragmentShader = VK_NULL_HANDLE;
        AV_UNREGISTER_SHADER_MODULE ( "ReflectionLocalProgram::_fragmentShader" )
    }

    if ( _vertexShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShader, nullptr );
    _vertexShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "ReflectionLocalProgram::_vertexShader" )
}

VkPipelineViewportStateCreateInfo const* ReflectionLocalProgram::InitViewportInfo (
    VkPipelineViewportStateCreateInfo &info,
    VkRect2D &scissorInfo,
    VkViewport &viewportInfo,
    VkExtent2D const &viewport
) const noexcept
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

VkPipelineVertexInputStateCreateInfo const* ReflectionLocalProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const noexcept
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
