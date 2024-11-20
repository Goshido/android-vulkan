#include <precompiled_headers.hpp>
#include <pbr/geometry_pass_bindings.inc>
#include <pbr/geometry_pass_program.hpp>
#include <vertex_info.hpp>


namespace pbr {

namespace {

constexpr char const* VERTEX_SHADER = "shaders/common_opaque.vs.spv";

constexpr size_t COLOR_RENDER_TARGET_COUNT = 4U;
constexpr size_t STAGE_COUNT = 2U;
constexpr size_t VERTEX_ATTRIBUTE_COUNT = 3U;
constexpr size_t VERTEX_INPUT_BINDING_COUNT = 2U;

// TBN64 format contains two quaternions
static_assert ( PBR_OPAQUE_MAX_INSTANCE_COUNT % 2 == 0 );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

GeometryPassProgram::ColorData::ColorData ( GXColorUNORM color0,
    GXColorUNORM color1,
    GXColorUNORM color2,
    GXColorUNORM emission,
    float emissionIntensity
) noexcept
{
    _emiRcol0rgb = static_cast<uint32_t> ( emission._data[ 0U ] ) |
        ( *reinterpret_cast<uint32_t const*> ( &color0 ) << 8U );

    _emiGcol1rgb = static_cast<uint32_t> ( emission._data[ 1U ] ) |
        ( *reinterpret_cast<uint32_t const*> ( &color1 ) << 8U );

    _emiBcol2rgb = static_cast<uint32_t> ( emission._data[ 2U ] ) |
        ( *reinterpret_cast<uint32_t const*> ( &color2 ) << 8U );

    // Emission intensity should take range from 0 to 6000.
    // Emission intensity is packed as 24bit fixed point value.
    constexpr double maxIntensity = 6.0e+3;
    constexpr double convertFactor = static_cast<double> ( 0x00FFFFFFU ) / maxIntensity;
    double const beta = convertFactor * std::clamp ( static_cast<double> ( emissionIntensity ), 0.0, maxIntensity );
    _col0aEmiIntens = static_cast<uint32_t> ( color0._data[ 3U ] ) | ( static_cast<uint32_t> ( beta ) << 8U );
}

//----------------------------------------------------------------------------------------------------------------------

GraphicsProgram::DescriptorSetInfo const &GeometryPassProgram::GetResourceInfo () const noexcept
{
    static DescriptorSetInfo const info =
    {
        {
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = 1U
            }
        },
        {
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = 5U
            },
        },
        {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 3U
            }
        }
    };

    return info;
}

void GeometryPassProgram::Destroy ( VkDevice device ) noexcept
{
    GraphicsProgram::Destroy ( device );

    _samplerLayout.Destroy ( device );
    _textureLayout.Destroy ( device );
    _instanceLayout.Destroy ( device );
}

bool GeometryPassProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
) noexcept
{
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    VkPipelineColorBlendAttachmentState attachmentInfo[ COLOR_RENDER_TARGET_COUNT ];
    VkVertexInputAttributeDescription attributeDescriptions[ VERTEX_ATTRIBUTE_COUNT ];
    VkVertexInputBindingDescription bindingDescription[ VERTEX_INPUT_BINDING_COUNT ];
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

    if ( !InitShaderInfo ( renderer, pipelineInfo.pStages, nullptr, nullptr, stageInfo ) ) [[unlikely]]
        return false;

    pipelineInfo.pVertexInputState = InitVertexInputInfo ( vertexInputInfo,
        attributeDescriptions,
        bindingDescription
    );

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

    char where[ 512U ];
    std::snprintf ( where, std::size ( where ), "%s::Init", _name.data () );

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        where,
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "%s", _name.data () )

    DestroyShaderModules ( device );
    return true;
}

void GeometryPassProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer,
    VkDescriptorSet const* sets,
    uint32_t startIndex,
    uint32_t count
) const noexcept
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

GeometryPassProgram::GeometryPassProgram ( std::string_view &&name, std::string_view &&fragmentShader ) noexcept:
    GraphicsProgram ( std::forward<std::string_view> ( name ) ),
    _fragmentShaderSource ( fragmentShader )
{
    // NOTHING
}

VkPipelineColorBlendStateCreateInfo const* GeometryPassProgram::InitColorBlendInfo (
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

        .colorWriteMask =
            AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
            AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT )
    };

    attachments[ 1U ] =
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

    attachments[ 2U ] =
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

    attachments[ 3U ] =
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

VkPipelineDepthStencilStateCreateInfo const* GeometryPassProgram::InitDepthStencilInfo (
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

VkPipelineDynamicStateCreateInfo const* GeometryPassProgram::InitDynamicStateInfo (
    VkPipelineDynamicStateCreateInfo* /*info*/
) const noexcept
{
    return nullptr;
}

VkPipelineInputAssemblyStateCreateInfo const* GeometryPassProgram::InitInputAssemblyInfo (
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

bool GeometryPassProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{

    if ( !_samplerLayout.Init ( device ) || !_textureLayout.Init ( device ) || !_instanceLayout.Init ( device ) )
    {
        [[unlikely]]
        return false;
    }

    VkDescriptorSetLayout const layouts[] =
    {
        _samplerLayout.GetLayout (),
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

    char where[ 512U ];
    std::snprintf ( where, std::size ( where ), "%s::InitLayout", _name.data () );

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        where,
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "%s", _name.data () )

    layout = _pipelineLayout;
    return true;
}

VkPipelineMultisampleStateCreateInfo const* GeometryPassProgram::InitMultisampleInfo (
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

VkPipelineRasterizationStateCreateInfo const* GeometryPassProgram::InitRasterizationInfo (
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

bool GeometryPassProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    VkPipelineShaderStageCreateInfo const* &targetInfo,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/,
    VkPipelineShaderStageCreateInfo* sourceInfo
) noexcept
{
    char where[ 512U ];
    std::snprintf ( where, std::size ( where ), "Can't create vertex shader (pbr::%s)", _name.data () );

    if ( !renderer.CreateShader ( _vertexShader, VERTEX_SHADER, where ) ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _vertexShader, VK_OBJECT_TYPE_SHADER_MODULE, VERTEX_SHADER )

    std::snprintf ( where, std::size ( where ), "Can't create fragment shader (pbr::%s)", _name.data () );

    if ( !renderer.CreateShader ( _fragmentShader, std::string ( _fragmentShaderSource ), where ) ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _fragmentShader,
        VK_OBJECT_TYPE_SHADER_MODULE,
        "%s",
        _fragmentShaderSource.data ()
    )

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

VkPipelineViewportStateCreateInfo const* GeometryPassProgram::InitViewportInfo (
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
        .offset =
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

VkPipelineVertexInputStateCreateInfo const* GeometryPassProgram::InitVertexInputInfo (
    VkPipelineVertexInputStateCreateInfo &info,
    VkVertexInputAttributeDescription* attributes,
    VkVertexInputBindingDescription* binds
) const noexcept
{
    binds[ IN_BUFFER_POSITION ] =
    {
        .binding = IN_BUFFER_POSITION,
        .stride = static_cast<uint32_t> ( sizeof ( GXVec3 ) ),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    binds[ IN_BUFFER_REST ] =
    {
        .binding = IN_BUFFER_REST,
        .stride = static_cast<uint32_t> ( sizeof ( android_vulkan::VertexInfo ) ),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    attributes[ IN_BUFFER_POSITION ] =
    {
        .location = IN_BUFFER_POSITION,
        .binding = IN_SLOT_POSITION,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0U
    };

    attributes[ IN_SLOT_UV ] =
    {
        .location = IN_SLOT_UV,
        .binding = IN_BUFFER_REST,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _uv ) )
    };

    attributes[ IN_SLOT_TBN ] =
    {
        .location = IN_SLOT_TBN,
        .binding = IN_BUFFER_REST,
        .format = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        .offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _tbn ) )
    };

    info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .vertexBindingDescriptionCount = VERTEX_INPUT_BINDING_COUNT,
        .pVertexBindingDescriptions = binds,
        .vertexAttributeDescriptionCount = static_cast<uint32_t> ( VERTEX_ATTRIBUTE_COUNT ),
        .pVertexAttributeDescriptions = attributes
    };

    return &info;
}

} // namespace pbr
