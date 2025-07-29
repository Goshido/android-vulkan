#include <precompiled_headers.hpp>
#include <pbr/brightness_factor.inc>
#include <pbr/tone_mapper_program.hpp>


namespace pbr {

namespace {

constexpr char const* VERTEX_SHADER = "shaders/full_screen_triangle.vs.spv";
constexpr char const* CUSTOM_BRIGHTNESS_FRAGMENT_SHADER = "shaders/tone_mapper_custom_brightness.ps.spv";
constexpr char const* DEFAULT_BRIGHTNESS_FRAGMENT_SHADER = "shaders/tone_mapper_default_brightness.ps.spv";

constexpr uint32_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ToneMapperProgram::ToneMapperProgram () noexcept:
    BrightnessProgram ( "pbr::ToneMapperProgram" )
{
    // NOTHING
}

void ToneMapperProgram::Destroy ( VkDevice device ) noexcept
{
    android::GraphicsProgram::Destroy ( device );

    _fullScreenTriangleLayout.Destroy ( device );
    _toneMapperLayout.Destroy ( device );
}

bool ToneMapperProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    BrightnessInfo const &brightnessInfo,
    VkExtent2D const &viewport
) noexcept
{
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo {};
    VkPipelineColorBlendAttachmentState attachmentInfo[ COLOR_RENDER_TARGET_COUNT ];
    VkPipelineColorBlendStateCreateInfo blendInfo {};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo {};
    VkPipelineMultisampleStateCreateInfo multisampleInfo {};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo {};
    VkRect2D scissorDescription {};
    VkPipelineShaderStageCreateInfo stageInfo[ STAGE_COUNT ];
    VkSpecializationInfo specInfo {};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    VkViewport viewportDescription {};
    VkPipelineViewportStateCreateInfo viewportInfo {};

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.stageCount = static_cast<uint32_t> ( STAGE_COUNT );

    VkDevice device = renderer.GetDevice ();

    if ( !InitShaderInfo ( renderer, pipelineInfo.pStages, &brightnessInfo, &specInfo, stageInfo ) ) [[unlikely]]
        return false;

    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo, nullptr, nullptr );
    pipelineInfo.pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo );
    pipelineInfo.pTessellationState = nullptr;

    pipelineInfo.pViewportState = InitViewportInfo ( viewportInfo,
        &scissorDescription,
        &viewportDescription,
        &viewport
    );

    pipelineInfo.pRasterizationState = InitRasterizationInfo ( rasterizationInfo );
    pipelineInfo.pMultisampleState = InitMultisampleInfo ( multisampleInfo );
    pipelineInfo.pDepthStencilState = InitDepthStencilInfo ( depthStencilInfo );
    pipelineInfo.pColorBlendState = InitColorBlendInfo ( blendInfo, attachmentInfo );
    pipelineInfo.pDynamicState = InitDynamicStateInfo ( nullptr );

    if ( !InitLayout ( device, pipelineInfo.layout ) ) [[unlikely]]
        return false;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::ToneMapperProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Tone mapper" )
    DestroyShaderModules ( device );
    return true;
}

void ToneMapperProgram::SetDescriptorSets ( VkCommandBuffer commandBuffer, VkDescriptorSet const* sets ) const noexcept
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        2U,
        sets,
        0U,
        nullptr
    );
}

VkPipelineColorBlendStateCreateInfo const* ToneMapperProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    *attachments =
    {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,

        .colorWriteMask =
            AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
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
        .attachmentCount = static_cast<uint32_t> ( COLOR_RENDER_TARGET_COUNT ),
        .pAttachments = attachments,
        .blendConstants = { 0.0F, 0.0F, 0.0F, 0.0F }
    };

    return &info;
}

VkPipelineDepthStencilStateCreateInfo const* ToneMapperProgram::InitDepthStencilInfo (
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
            .writeMask = 0x00U,
            .reference = std::numeric_limits<uint32_t>::max ()
        },

        .back =
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

VkPipelineDynamicStateCreateInfo const* ToneMapperProgram::InitDynamicStateInfo (
    VkPipelineDynamicStateCreateInfo* /*info*/
) const noexcept
{
    return nullptr;
}

VkPipelineInputAssemblyStateCreateInfo const* ToneMapperProgram::InitInputAssemblyInfo (
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

bool ToneMapperProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_fullScreenTriangleLayout.Init ( device ) || !_toneMapperLayout.Init ( device ) ) [[unlikely]]
        return false;

    VkDescriptorSetLayout const layouts[] =
    {
        _fullScreenTriangleLayout.GetLayout (),
        _toneMapperLayout.GetLayout ()
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
        "pbr::ToneMapperProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Tone mapper" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* ToneMapperProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* ToneMapperProgram::InitRasterizationInfo (
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

bool ToneMapperProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    SpecializationData specializationData,
    VkSpecializationInfo* specializationInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
) noexcept
{
    bool result = renderer.CreateShader ( _vertexShader,
        VERTEX_SHADER,
        "Can't create vertex shader (pbr::ToneMapperProgram)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _vertexShader, VK_OBJECT_TYPE_SHADER_MODULE, VERTEX_SHADER )

    constexpr char const* const cases[] = { CUSTOM_BRIGHTNESS_FRAGMENT_SHADER, DEFAULT_BRIGHTNESS_FRAGMENT_SHADER };
    auto const &info = *static_cast<BrightnessInfo const*> ( specializationData );
    char const* const fs = cases[ static_cast<size_t> ( info._isDefaultBrightness ) ];
    result = renderer.CreateShader ( _fragmentShader, fs, "Can't create fragment shader (pbr::ToneMapperProgram)" );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _fragmentShader, VK_OBJECT_TYPE_SHADER_MODULE, "%s", fs )

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

    constexpr static VkSpecializationMapEntry entry
    {
        .constantID = CONST_BRIGHTNESS_FACTOR,
        .offset = static_cast<uint32_t> ( offsetof ( BrightnessInfo, _brightnessFactor ) ),
        .size = sizeof ( BrightnessInfo::_brightnessFactor )
    };

    *specializationInfo =
    {
        .mapEntryCount = 1U,
        .pMapEntries = &entry,
        .dataSize = sizeof ( BrightnessInfo ),
        .pData = specializationData
    };

    sourceInfo[ 1U ] =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = _fragmentShader,
        .pName = FRAGMENT_SHADER_ENTRY_POINT,
        .pSpecializationInfo = specializationInfo
    };

    targetInfo = sourceInfo;
    return true;
}

VkPipelineViewportStateCreateInfo const* ToneMapperProgram::InitViewportInfo ( VkPipelineViewportStateCreateInfo &info,
    VkRect2D* scissorInfo,
    VkViewport* viewportInfo,
    VkExtent2D const* viewport
) const noexcept
{
    *viewportInfo =
    {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float> ( viewport->width ),
        .height = static_cast<float> ( viewport->height ),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };

    *scissorInfo =
    {
        .offset
        {
            .x = 0,
            .y = 0
        },

        .extent = *viewport
    };

    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .viewportCount = 1U,
        .pViewports = viewportInfo,
        .scissorCount = 1U,
        .pScissors = scissorInfo
    };

    return &info;
}

VkPipelineVertexInputStateCreateInfo const* ToneMapperProgram::InitVertexInputInfo (
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

} // namespace pbr
