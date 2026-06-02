#include <precompiled_headers.hpp>
#include <file.hpp>
#include <platform/windows/pbr/opaque_program.hpp>
#include <platform/windows/pbr/gbuffer_pass_binds.inc>


namespace pbr {

namespace {

constexpr char const VERTEX_SHADER[] = "shaders/windows/gbuffer_mesh.vs.spv";
constexpr char const FRAGMENT_SHADER[] = "shaders/windows/opaque.ps.spv";

constexpr size_t COLOR_RENDER_TARGET_COUNT = 4U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

OpaqueProgram::OpaqueProgram () noexcept:
    GraphicsProgram ( "Opaque", sizeof ( PushConstants ) )
{
    // NOTHING
}

void OpaqueProgram::Destroy ( VkDevice device ) noexcept
{
    GraphicsProgram::Destroy ( device );
    _layout.Destroy ( device );
}

bool OpaqueProgram::Init ( VkDevice device, VkFormat depthStencilFormat ) noexcept
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
    VkRect2D scissorDescription {};
    VkPipelineShaderStageCreateInfo stageInfo[ STAGE_COUNT ];
    VkSpecializationInfo specInfo {};
    VkViewport viewportDescription {};
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
        .stageCount = static_cast<uint32_t> ( STAGE_COUNT )
    };

    bool result = InitShaderInfo ( pipelineInfo.pStages,
        vs,
        fs,
        nullptr,
        nullptr,
        moduleInfo,
        stageInfo
    );

    if ( !result ) [[unlikely]]
        return false;

    pipelineInfo.pVertexInputState = InitVertexInputInfo ();
    pipelineInfo.pInputAssemblyState = InitInputAssemblyInfo ( assemblyInfo );
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = InitViewportInfo ( viewportInfo, nullptr, nullptr, nullptr );
    pipelineInfo.pRasterizationState = InitRasterizationInfo ( rasterizationInfo );
    pipelineInfo.pMultisampleState = InitMultisampleInfo ( multisampleInfo );
    pipelineInfo.pDepthStencilState = InitDepthStencilInfo ( depthStencilInfo );
    pipelineInfo.pColorBlendState = InitColorBlendInfo ( blendInfo, attachmentInfo );
    pipelineInfo.pDynamicState = InitDynamicStateInfo ( &dynamicStateInfo );

    if ( !InitLayout ( device, pipelineInfo.layout ) ) [[unlikely]]
        return false;

    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0U;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::OpaqueProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Opaque" )
    return true;
}

VkPipelineColorBlendStateCreateInfo const* OpaqueProgram::InitColorBlendInfo (
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

VkPipelineDepthStencilStateCreateInfo const* OpaqueProgram::InitDepthStencilInfo (
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

VkPipelineDynamicStateCreateInfo const* OpaqueProgram::InitDynamicStateInfo (
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

VkPipelineInputAssemblyStateCreateInfo const* OpaqueProgram::InitInputAssemblyInfo (
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

bool OpaqueProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    constexpr VkPushConstantRange pushConstantRange
    {
        .stageFlags = AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT ),
        .offset = 0U,
        .size = static_cast<uint32_t> ( sizeof ( PushConstants ) )
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
        "pbr::OpaqueProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Opaque" )
    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* OpaqueProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* OpaqueProgram::InitRasterizationInfo (
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

VkPipelineViewportStateCreateInfo const* OpaqueProgram::InitViewportInfo ( VkPipelineViewportStateCreateInfo &info,
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

VkPipelineRenderingCreateInfo const* OpaqueProgram::InitRenderingInfo ( VkFormat /*nativeColor*/,
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
        .stencilAttachmentFormat = nativeDepthStencil
    };

    return &info;
}

bool OpaqueProgram::InitShaderInfo ( VkPipelineShaderStageCreateInfo const* &targetInfo,
    std::vector<uint8_t> &vs,
    std::vector<uint8_t> &fs,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/,
    VkShaderModuleCreateInfo* moduleInfo,
    VkPipelineShaderStageCreateInfo* sourceInfo
) const noexcept
{
    android_vulkan::File vsFile ( VERTEX_SHADER );
    android_vulkan::File fsFile ( FRAGMENT_SHADER );

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

    targetInfo = sourceInfo;
    return true;
}

} // namespace pbr
