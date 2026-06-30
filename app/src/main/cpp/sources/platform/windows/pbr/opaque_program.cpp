#include <precompiled_headers.hpp>
#include <platform/windows/pbr/gbuffer_pass_binds.inc>
#include <platform/windows/pbr/opaque_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <renderer.hpp>


namespace pbr {

namespace {

constexpr size_t COLOR_RENDER_TARGET_COUNT = 4U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

OpaqueProgram::OpaqueProgram () noexcept:
    GBufferProgram ( "shaders/windows/gbuffer_mesh.vs.spv", "shaders/windows/opaque.ps.spv", sizeof ( PushConstants ) )
{
    // NOTHING
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
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    return &info;
}

} // namespace pbr
