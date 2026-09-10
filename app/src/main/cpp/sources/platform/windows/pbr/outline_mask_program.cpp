#include <precompiled_headers.hpp>
#include <file.hpp>
#include <platform/windows/pbr/outline_mask_binds.inc>
#include <platform/windows/pbr/outline_mask_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <renderer.hpp>


namespace pbr {

namespace {

constexpr char const VERTEX_SHADER[] = "shaders/windows/outline_mask.vs.spv";
constexpr char const FRAGMENT_SHADER[] = "shaders/windows/outline_mask.ps.spv";

constexpr size_t COLOR_RENDER_TARGET_COUNT = 1U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

OutlineMaskProgram::OutlineMaskProgram () noexcept:
    GraphicsProgram ( sizeof ( PushConstants ) )
{
    // NOTHING
}

bool OutlineMaskProgram::Init ( VkDevice device, VkFormat depthStencilFormat ) noexcept
{
    VkFormat colorFormats[] =
    {
        VK_FORMAT_R8_UNORM
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

    VkGraphicsPipelineCreateInfo const pipelineInfo
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
        .layout = UniversalPipelineLayout::GetPipelineLayout (),
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0U,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::OutlineMaskProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Outline mask" )
    return true;
}

VkPipelineDepthStencilStateCreateInfo const* OutlineMaskProgram::InitDepthStencilInfo (
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

VkPipelineDynamicStateCreateInfo const* OutlineMaskProgram::InitDynamicStateInfo (
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

VkPipelineInputAssemblyStateCreateInfo const* OutlineMaskProgram::InitInputAssemblyInfo (
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

VkPipelineMultisampleStateCreateInfo const* OutlineMaskProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* OutlineMaskProgram::InitRasterizationInfo (
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

VkPipelineViewportStateCreateInfo const* OutlineMaskProgram::InitViewportInfo ( VkPipelineViewportStateCreateInfo &info,
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

VkPipelineShaderStageCreateInfo const* OutlineMaskProgram::InitShaderInfo ( std::vector<uint8_t> &vs,
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

VkPipelineColorBlendStateCreateInfo const* OutlineMaskProgram::InitColorBlendInfo (
    VkPipelineColorBlendStateCreateInfo &info,
    VkPipelineColorBlendAttachmentState* attachments
) const noexcept
{
    attachments[ OUT_MASK ] =
    {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
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

VkPipelineRenderingCreateInfo const* OutlineMaskProgram::InitRenderingInfo ( VkFormat /*nativeColor*/,
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

} // namespace pbr
