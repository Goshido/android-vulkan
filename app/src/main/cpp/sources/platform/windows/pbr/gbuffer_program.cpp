#include <precompiled_headers.hpp>
#include <file.hpp>
#include <platform/windows/pbr/gbuffer_pass_binds.inc>
#include <platform/windows/pbr/gbuffer_program.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t COLOR_RENDER_TARGET_COUNT = 4U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void GBufferProgram::Destroy ( VkDevice device ) noexcept
{
    GraphicsProgram::Destroy ( device );
    _layout.Destroy ( device );
}

bool GBufferProgram::Init ( VkDevice device, VkFormat depthStencilFormat ) noexcept
{
    VkFormat colorFormats[] =
    {
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        VK_FORMAT_R8G8B8A8_UNORM
    };

    static_assert ( std::size ( colorFormats ) == COLOR_RENDER_TARGET_COUNT );

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo {};
    VkPipelineColorBlendAttachmentState attachmentInfo[ std::size ( colorFormats ) ];
    VkPipelineColorBlendStateCreateInfo blendInfo {};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo {};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
    VkShaderModuleCreateInfo moduleInfo[ STAGE_COUNT ];
    VkPipelineMultisampleStateCreateInfo multisampleInfo {};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo {};
    VkPipelineRenderingCreateInfo renderingInfo {};
    VkPipelineShaderStageCreateInfo stageInfo[ STAGE_COUNT ];
    VkSpecializationInfo specInfo {};
    VkPipelineViewportStateCreateInfo viewportInfo {};
    std::vector<uint8_t> vs {};
    std::vector<uint8_t> fs {};

    VkGraphicsPipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

        .pNext = InitRenderingInfo ( VK_FORMAT_UNDEFINED,
            VK_FORMAT_UNDEFINED,
            VK_FORMAT_UNDEFINED,
            depthStencilFormat,
            colorFormats,
            renderingInfo
        ),

        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .stageCount = static_cast<uint32_t> ( STAGE_COUNT ),
        .pStages = InitShaderInfo ( vs, fs, nullptr, nullptr, moduleInfo, stageInfo ),
        .pVertexInputState = InitVertexInputInfo (),
        .pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo ),
        .pTessellationState = nullptr,
        .pViewportState = InitViewportInfo ( viewportInfo, nullptr, nullptr, nullptr ),
        .pRasterizationState = InitRasterizationInfo ( rasterizationInfo ),
        .pMultisampleState = InitMultisampleInfo ( multisampleInfo ),
        .pDepthStencilState = InitDepthStencilInfo ( depthStencilInfo ),
        .pColorBlendState = InitColorBlendInfo ( blendInfo, attachmentInfo ),
        .pDynamicState = InitDynamicStateInfo ( &dynamicStateInfo ),
        .layout = InitLayout ( device ),
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0U,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::GBufferProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "%s", _name.data () )
    return true;
}

GBufferProgram::GBufferProgram ( std::string_view vs,
    std::string_view fs,
    std::string_view name,
    size_t pushConstantSize
) noexcept:
    GraphicsProgram ( pushConstantSize ),
    _vsSource ( vs ),
    _fsSource ( fs ),
    _name ( name )
{
    // NOTHING
}

VkPipelineColorBlendStateCreateInfo const* GBufferProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    constexpr VkPipelineColorBlendAttachmentState state
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

    attachments[ OUT_ALBEDO ] = state;
    attachments[ OUT_EMISSION ] = state;
    attachments[ OUT_NORMAL ] = state;
    attachments[ OUT_PARAM ] = state;

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

VkPipelineDepthStencilStateCreateInfo const* GBufferProgram::InitDepthStencilInfo (
    VkPipelineDepthStencilStateCreateInfo &info
) const noexcept
{
    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_GREATER,
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

VkPipelineDynamicStateCreateInfo const* GBufferProgram::InitDynamicStateInfo (
    VkPipelineDynamicStateCreateInfo* info
) const noexcept
{
    constexpr static VkDynamicState const states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

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

VkPipelineInputAssemblyStateCreateInfo const* GBufferProgram::InitInputAssemblyInfo (
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

VkPipelineLayout GBufferProgram::InitLayout ( VkDevice device ) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return VK_NULL_HANDLE;

    VkPushConstantRange const pushConstantRange
    {
        .stageFlags = AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT ),
        .offset = 0U,
        .size = _pushConstantSize
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
        "pbr::GBufferProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return VK_NULL_HANDLE;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "%s", _name.data () )
    return _pipelineLayout;
}

VkPipelineMultisampleStateCreateInfo const* GBufferProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* GBufferProgram::InitRasterizationInfo (
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
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0F,
        .depthBiasClamp = 0.0F,
        .depthBiasSlopeFactor = 0.0F,
        .lineWidth = 1.0F
    };

    return &info;
}

VkPipelineViewportStateCreateInfo const* GBufferProgram::InitViewportInfo ( VkPipelineViewportStateCreateInfo &info,
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

VkPipelineRenderingCreateInfo const* GBufferProgram::InitRenderingInfo ( VkFormat /*nativeColor*/,
    VkFormat /*nativeDepth*/,
    VkFormat /*nativeStencil*/,
    VkFormat nativeDepthStencil,
    VkFormat* colorAttachments,
    VkPipelineRenderingCreateInfo &info
) const noexcept
{
    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0U,
        .colorAttachmentCount = static_cast<uint32_t> ( COLOR_RENDER_TARGET_COUNT ),
        .pColorAttachmentFormats = colorAttachments,
        .depthAttachmentFormat = nativeDepthStencil,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    return &info;
}

VkPipelineShaderStageCreateInfo const* GBufferProgram::InitShaderInfo ( std::vector<uint8_t> &vs,
    std::vector<uint8_t> &fs,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/,
    VkShaderModuleCreateInfo* moduleInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
) const noexcept
{
    android_vulkan::File vsFile ( _vsSource );
    android_vulkan::File fsFile ( _fsSource );

    if ( !vsFile.LoadContent () || !fsFile.LoadContent () ) [[unlikely]]
        return nullptr;

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

    sourceInfo[ 1U ] =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = moduleInfo + 1U,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = VK_NULL_HANDLE,
        .pName = FRAGMENT_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };

    return sourceInfo;
}

} // namespace pbr
