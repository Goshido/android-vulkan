#include <precompiled_headers.hpp>
#include <file.hpp>
#include <platform/windows/pbr/ui_program.hpp>
#include <pbr/brightness_factor.inc>


// FUCK - remove namespace
namespace pbr::windows {

namespace {

constexpr char const* VERTEX_SHADER = "shaders/windows/ui.vs.spv";
constexpr char const* CUSTOM_BRIGHTNESS_FRAGMENT_SHADER = "shaders/windows/ui_custom_brightness.ps.spv";
constexpr char const* DEFAULT_BRIGHTNESS_FRAGMENT_SHADER = "shaders/windows/ui_default_brightness.ps.spv";

constexpr uint32_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UIProgram::UIProgram () noexcept:
    windows::GraphicsProgram ( "UI", sizeof ( UIInfo ) )
{
    // NOTHING
}

void UIProgram::Destroy ( VkDevice device ) noexcept
{
    windows::GraphicsProgram::Destroy ( device );
    _layout.Destroy ( device );
}

bool UIProgram::Init ( VkDevice device,
    VkFormat swapchainFormat,
    BrightnessInfo const &brightnessInfo,
    VkExtent2D const &viewport
) noexcept
{
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo {};
    VkPipelineColorBlendAttachmentState attachmentInfo[ COLOR_RENDER_TARGET_COUNT ];
    VkPipelineColorBlendStateCreateInfo blendInfo {};
    VkFormat colorAttachment;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo {};
    VkShaderModuleCreateInfo moduleInfo[ STAGE_COUNT ];
    VkPipelineMultisampleStateCreateInfo multisampleInfo {};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo {};
    VkPipelineRenderingCreateInfo renderingInfo {};
    VkRect2D scissorDescription {};
    VkPipelineShaderStageCreateInfo stageInfo[ STAGE_COUNT ];
    VkSpecializationInfo specInfo {};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    VkViewport viewportDescription {};
    VkPipelineViewportStateCreateInfo viewportInfo {};
    std::vector<uint8_t> vs{};
    std::vector<uint8_t> fs{};

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    pipelineInfo.pNext = InitRenderingInfo ( swapchainFormat,
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_UNDEFINED,
        &colorAttachment,
        renderingInfo
    );

    pipelineInfo.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipelineInfo.stageCount = static_cast<uint32_t> ( STAGE_COUNT );

    bool result = InitShaderInfo ( pipelineInfo.pStages,
        vs,
        fs,
        &brightnessInfo,
        &specInfo,
        moduleInfo,
        stageInfo
    );

    if ( !result ) [[unlikely]]
        return false;

    pipelineInfo.pVertexInputState = InitVertexInputInfo ();
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

    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0U;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::windows::UIProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "UI" )
    return true;
}

VkPipelineColorBlendStateCreateInfo const* UIProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    *attachments =
    {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
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

VkPipelineDepthStencilStateCreateInfo const* UIProgram::InitDepthStencilInfo (
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

VkPipelineDynamicStateCreateInfo const* UIProgram::InitDynamicStateInfo (
    VkPipelineDynamicStateCreateInfo* /*info*/
) const noexcept
{
    return nullptr;
}

VkPipelineInputAssemblyStateCreateInfo const* UIProgram::InitInputAssemblyInfo (
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

bool UIProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    constexpr VkPushConstantRange pushConstantRange
    {
        .stageFlags = AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT ),
        .offset = 0U,
        .size = static_cast<uint32_t> ( sizeof ( UIInfo ) )
    };

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &_layout.GetLayout (),
        .pushConstantRangeCount = 1U,
        .pPushConstantRanges = &pushConstantRange
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::windows::UIProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "UI" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* UIProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* UIProgram::InitRasterizationInfo (
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

VkPipelineViewportStateCreateInfo const* UIProgram::InitViewportInfo (
    VkPipelineViewportStateCreateInfo &info,
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

VkPipelineRenderingCreateInfo const* UIProgram::InitRenderingInfo ( VkFormat nativeColor,
    VkFormat /*nativeDepth*/,
    VkFormat /*nativeStencil*/,
    VkFormat /*nativeDepthStencil*/,
    VkFormat* colorAttachments,
    VkPipelineRenderingCreateInfo &info
) const noexcept
{
    *colorAttachments = nativeColor;

    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0U,
        .colorAttachmentCount = COLOR_RENDER_TARGET_COUNT,
        .pColorAttachmentFormats = colorAttachments,
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    return &info;
}

bool UIProgram::InitShaderInfo ( VkPipelineShaderStageCreateInfo const* &targetInfo,
    std::vector<uint8_t> &vs,
    std::vector<uint8_t> &fs,
    SpecializationData specializationData,
    VkSpecializationInfo* specializationInfo,
    VkShaderModuleCreateInfo* moduleInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
) const noexcept
{
    constexpr char const* const cases[] = { CUSTOM_BRIGHTNESS_FRAGMENT_SHADER, DEFAULT_BRIGHTNESS_FRAGMENT_SHADER };
    auto const &info = *static_cast<BrightnessInfo const*> ( specializationData );
    android_vulkan::File vsFile ( VERTEX_SHADER );
    android_vulkan::File fsFile ( cases[ static_cast<size_t> ( info._isDefaultBrightness ) ] );

    if ( !vsFile.LoadContent () || !fsFile.LoadContent () ) [[unlikely]]
        return false;

    vs = std::move ( vsFile.GetContent () );
    fs = std::move ( fsFile.GetContent () );

    moduleInfo[ 0U ] =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .codeSize = vs.size (),
        .pCode = reinterpret_cast<uint32_t const*> ( vs.data () )
    };

    sourceInfo[ 0U ] =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = moduleInfo,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = VK_NULL_HANDLE,
        .pName = VERTEX_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };

    moduleInfo[ 1U ] =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .codeSize = fs.size (),
        .pCode = reinterpret_cast<uint32_t const*> ( fs.data () )
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
        .pNext = moduleInfo + 1U,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = VK_NULL_HANDLE,
        .pName = FRAGMENT_SHADER_ENTRY_POINT,
        .pSpecializationInfo = specializationInfo
    };

    targetInfo = sourceInfo;
    return true;
}

} // namespace pbr
