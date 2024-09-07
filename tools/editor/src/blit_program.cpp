#include <blit_program.hpp>
#include <vulkan_utils.hpp>


namespace editor {

namespace {

constexpr char const* VERTEX_SHADER = "../editor-assets/shaders/blit.vs.spv";
constexpr char const* FRAGMENT_SHADER = "../editor-assets/shaders/blit.ps.spv";

constexpr uint32_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

BlitProgram::BlitProgram () noexcept:
    pbr::GraphicsProgram ( "editor::BlitProgram" )
{
    // NOTHING
}

pbr::GraphicsProgram::DescriptorSetInfo const &BlitProgram::GetResourceInfo () const noexcept
{
    static DescriptorSetInfo const info
    {
        {
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = 1U
            },
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = 1U
            }
        }
    };

    return info;
}

void BlitProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
    }

    _blitLayout.Destroy ( device );

    if ( _pipeline != VK_NULL_HANDLE )
    {
        vkDestroyPipeline ( device, _pipeline, nullptr );
        _pipeline = VK_NULL_HANDLE;
    }

    DestroyShaderModules ( device );
}

bool BlitProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass
) noexcept
{
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    VkPipelineColorBlendAttachmentState attachmentInfo[ COLOR_RENDER_TARGET_COUNT ];
    VkPipelineColorBlendStateCreateInfo blendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineShaderStageCreateInfo stageInfo[ STAGE_COUNT ];
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineViewportStateCreateInfo viewportInfo;

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.stageCount = static_cast<uint32_t> ( STAGE_COUNT );

    VkDevice device = renderer.GetDevice ();

    if ( !InitShaderInfo ( renderer, pipelineInfo.pStages, nullptr, nullptr, stageInfo ) ) [[unlikely]]
        return false;

    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo, nullptr, nullptr );
    pipelineInfo.pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo );
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = InitViewportInfo ( viewportInfo, nullptr, nullptr, nullptr );
    pipelineInfo.pRasterizationState = InitRasterizationInfo ( rasterizationInfo );
    pipelineInfo.pMultisampleState = InitMultisampleInfo ( multisampleInfo );
    pipelineInfo.pDepthStencilState = InitDepthStencilInfo ( depthStencilInfo );
    pipelineInfo.pColorBlendState = InitColorBlendInfo ( blendInfo, attachmentInfo );
    pipelineInfo.pDynamicState = InitDynamicStateInfo ( & dynamicStateInfo );

    if ( !InitLayout ( device, pipelineInfo.layout ) ) [[unlikely]]
        return false;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "editor::BlitProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Blit" )
    DestroyShaderModules ( device );
    return true;
}

void BlitProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer,
    VkDescriptorSet set
) const noexcept
{
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

VkPipelineColorBlendStateCreateInfo const* BlitProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    attachments[ 0U ] =
    {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,

        .colorWriteMask = AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT )
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

VkPipelineDepthStencilStateCreateInfo const* BlitProgram::InitDepthStencilInfo (
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

        .front
        {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = std::numeric_limits<uint32_t>::max (),
            .writeMask = 0x00U,
            .reference = std::numeric_limits<uint32_t>::max ()
        },

        .back
        {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = std::numeric_limits<uint32_t>::max (),
            .writeMask = 0x00U,
            .reference = std::numeric_limits<uint32_t>::max ()
        },

        .minDepthBounds = 0.0F,
        .maxDepthBounds = 1.0F
    };

    return &info;
}

VkPipelineDynamicStateCreateInfo const* BlitProgram::InitDynamicStateInfo (
    VkPipelineDynamicStateCreateInfo* info
) const noexcept
{
    constexpr static VkDynamicState const states [] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    *info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .dynamicStateCount = static_cast<uint32_t> ( std::size ( states ) ),
        .pDynamicStates = states
    };

    return info;
}

VkPipelineInputAssemblyStateCreateInfo const* BlitProgram::InitInputAssemblyInfo (
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

bool BlitProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_blitLayout.Init ( device ) ) [[unlikely]]
        return false;

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &_blitLayout.GetLayout (),
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "editor::BlitProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _pipelineLayout,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        "Blit"
    )

    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* BlitProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* BlitProgram::InitRasterizationInfo (
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
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0F,
        .depthBiasClamp = 0.0F,
        .depthBiasSlopeFactor = 0.0F,
        .lineWidth = 1.0F
    };

    return &info;
}

bool BlitProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/,
    VkPipelineShaderStageCreateInfo* sourceInfo
) noexcept
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (editor::BlitProgram)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _vertexShader, VK_OBJECT_TYPE_SHADER_MODULE, VERTEX_SHADER )

    result = renderer.CreateShader ( _fragmentShader,
        FRAGMENT_SHADER,
        "Can't create fragment shader (editor::BlitProgram)"
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
        .pName = pbr::VERTEX_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };

    sourceInfo[ 1U ] =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = _fragmentShader,
        .pName = pbr::FRAGMENT_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };

    targetInfo = sourceInfo;
    return true;
}

void BlitProgram::DestroyShaderModules ( VkDevice device ) noexcept
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

VkPipelineViewportStateCreateInfo const* BlitProgram::InitViewportInfo (
    VkPipelineViewportStateCreateInfo &info,
    VkRect2D* /*scissorInfo*/,
    VkViewport* /*viewportInfo*/,
    VkExtent2D const* /*viewport*/
) const noexcept
{
    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .viewportCount = 1U,
        .pViewports = nullptr,
        .scissorCount = 1U,
        .pScissors = nullptr
    };

    return &info;
}

VkPipelineVertexInputStateCreateInfo const* BlitProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* /*attributes*/,
    VkVertexInputBindingDescription* /*binds*/
) const noexcept
{
    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .vertexBindingDescriptionCount = 0U,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0U,
        .pVertexAttributeDescriptions = nullptr
    };

    return &info;
}

} // namespace editor
