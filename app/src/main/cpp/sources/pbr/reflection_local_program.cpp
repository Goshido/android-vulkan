#include <pbr/light_volume.inc>
#include <pbr/reflection_local_program.hpp>


namespace pbr {

namespace {

constexpr size_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr size_t STAGE_COUNT = 2U;
constexpr size_t VERTEX_ATTRIBUTE_COUNT = 1U;

constexpr char const* VERTEX_SHADER = "shaders/light_volume.vs.spv";
constexpr char const* FRAGMENT_SHADER = "shaders/reflection_local.ps.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ReflectionLocalProgram::ReflectionLocalProgram () noexcept:
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
    SpecializationData /*specializationData*/,
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

    if ( !InitShaderInfo ( renderer, pipelineInfo.pStages, nullptr, nullptr, stageInfo ) ) [[unlikely]]
        return false;

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

    if ( !InitLayout ( device, pipelineInfo.layout ) ) [[unlikely]]
        return false;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::ReflectionLocalProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "pbr::ReflectionLocalProgram::_pipeline" )
    DestroyShaderModules ( device );
    return true;
}

void ReflectionLocalProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
    }

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
    }

    _reflectionLayout.Destroy ( device );
    _lightVolumeLayout.Destroy ( device );
    _commonLayout.Destroy ( device );

    DestroyShaderModules ( device );
}

GraphicsProgram::DescriptorSetInfo const &ReflectionLocalProgram::GetResourceInfo () const noexcept
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

void ReflectionLocalProgram::SetLightData ( VkCommandBuffer commandBuffer,
    VkDescriptorSet transform,
    VkDescriptorSet lightData
) const noexcept
{
    VkDescriptorSet const sets[] = { transform, lightData };

    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        1U,
        static_cast<uint32_t> ( std::size ( sets ) ),
        sets,
        0U,
        nullptr
    );
}

VkPipelineColorBlendStateCreateInfo const* ReflectionLocalProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    attachments[ 0U ] =
    {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,

        .colorWriteMask = AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT )
    };

    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_NO_OP,
        .attachmentCount = COLOR_RENDER_TARGET_COUNT,
        .pAttachments = attachments,
        .blendConstants = { 0.0F, 0.0F, 0.0F, 0.0F }
    };

    return &info;
}

VkPipelineDepthStencilStateCreateInfo const* ReflectionLocalProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const noexcept
{
    info =
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
            .reference = 0U
        },

        .back =
        {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = std::numeric_limits<uint32_t>::max (),
            .writeMask = 0U,
            .reference = 0U
        },

        .minDepthBounds = 0.0F,
        .maxDepthBounds = 1.0F
    };

    return &info;
}

VkPipelineInputAssemblyStateCreateInfo const* ReflectionLocalProgram::InitInputAssemblyInfo (
    VkPipelineInputAssemblyStateCreateInfo &info
) const noexcept
{
    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    return &info;
}

bool ReflectionLocalProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_commonLayout.Init ( device ) || !_lightVolumeLayout.Init ( device ) || !_reflectionLayout.Init ( device ) )
    {
        [[unlikely]]
        return false;
    }

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
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::ReflectionLocalProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _pipelineLayout,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        "pbr::ReflectionLocalProgram::_pipelineLayout"
    )

    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* ReflectionLocalProgram::InitMultisampleInfo (
    VkPipelineMultisampleStateCreateInfo &info
) const noexcept
{
    info =
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

    return &info;
}

VkPipelineRasterizationStateCreateInfo const* ReflectionLocalProgram::InitRasterizationInfo (
    VkPipelineRasterizationStateCreateInfo &info
) const noexcept
{
    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_FRONT_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0F,
        .depthBiasClamp = 0.0F,
        .depthBiasSlopeFactor = 0.0F,
        .lineWidth = 1.0F
    };

    return &info;
}

bool ReflectionLocalProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/,
    VkPipelineShaderStageCreateInfo* sourceInfo
) noexcept
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::ReflectionLocalProgram)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _vertexShader, VK_OBJECT_TYPE_SHADER_MODULE, VERTEX_SHADER )

    result = renderer.CreateShader ( _fragmentShader,
        FRAGMENT_SHADER,
        "Can't create fragment shader (pbr::ReflectionLocalProgram)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _fragmentShader, VK_OBJECT_TYPE_SHADER_MODULE, FRAGMENT_SHADER )

    sourceInfo[ 0U ] =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = _vertexShader,
        .pName = VERTEX_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };

    sourceInfo[ 1U ] =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = _fragmentShader,
        .pName = FRAGMENT_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };

    targetInfo = sourceInfo;
    return true;
}

void ReflectionLocalProgram::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _fragmentShader != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, _fragmentShader, nullptr );
        _fragmentShader = VK_NULL_HANDLE;
    }

    if ( _vertexShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShader, nullptr );
    _vertexShader = VK_NULL_HANDLE;
}

VkPipelineViewportStateCreateInfo const* ReflectionLocalProgram::InitViewportInfo (
    VkPipelineViewportStateCreateInfo &info,
    VkRect2D &scissorInfo,
    VkViewport &viewportInfo,
    VkExtent2D const &viewport
) const noexcept
{
    viewportInfo =
    {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float> ( viewport.width ),
        .height = static_cast<float> ( viewport.height ),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };

    scissorInfo =
    {
        .offset =
        {
            .x = 0,
            .y = 0,
        },

        .extent = viewport
    };

    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .viewportCount = 1U,
        .pViewports = &viewportInfo,
        .scissorCount = 1U,
        .pScissors = &scissorInfo
    };

    return &info;
}

VkPipelineVertexInputStateCreateInfo const* ReflectionLocalProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const noexcept
{
    binds[ 0U ] =
    {
        .binding = 0U,
        .stride = static_cast<uint32_t> ( sizeof ( GXVec3 ) ),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    attributes[ 0U ] =
    {
        .location = IN_SLOT_VERTEX,
        .binding = 0U,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0U
    };

    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .vertexBindingDescriptionCount = 1U,
        .pVertexBindingDescriptions = binds,
        .vertexAttributeDescriptionCount = static_cast<uint32_t> ( VERTEX_ATTRIBUTE_COUNT ),
        .pVertexAttributeDescriptions = attributes
    };

    return &info;
}

} // namespace pbr
