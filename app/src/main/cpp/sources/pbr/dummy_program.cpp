#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/dummy_program.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr char const* VERTEX_SHADER = "shaders/dummy.vs.spv";

constexpr size_t MAX_COLOR_RENDER_TARGET_COUNT = 4U;
constexpr size_t STAGE_COUNT = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

GraphicsProgram::DescriptorSetInfo const &DummyProgram::GetResourceInfo () const noexcept
{
    static DescriptorSetInfo const info {};
    return info;
}

bool DummyProgram::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    VkExtent2D const &viewport
) noexcept
{
    AV_ASSERT ( _colorRenderTargets <= MAX_COLOR_RENDER_TARGET_COUNT )

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    VkPipelineColorBlendAttachmentState attachmentInfo[ MAX_COLOR_RENDER_TARGET_COUNT ];
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
    pipelineInfo.subpass = _subpass;
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



DummyProgram::DummyProgram ( std::string_view &&name,
    std::string_view &&fragmentShader,
    uint32_t colorRenderTargets,
    uint32_t subpass
) noexcept:
    GraphicsProgram ( std::forward<std::string_view> ( name ) ),
    _colorRenderTargets ( colorRenderTargets ),
    _fragmentShaderSource ( fragmentShader ),
    _subpass ( subpass )
{
    // NOTHING
}

bool DummyProgram::InitLayoutInternal ( VkDevice device,
    VkPipelineLayout &layout,
    VkDescriptorSetLayout descriptorSetLayout
) noexcept
{
    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &descriptorSetLayout,
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

VkPipelineDynamicStateCreateInfo const* DummyProgram::InitDynamicStateInfo (
    VkPipelineDynamicStateCreateInfo* /*info*/
) const noexcept
{
    return nullptr;
}

VkPipelineInputAssemblyStateCreateInfo const* DummyProgram::InitInputAssemblyInfo (
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

VkPipelineMultisampleStateCreateInfo const* DummyProgram::InitMultisampleInfo (
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

bool DummyProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
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

VkPipelineViewportStateCreateInfo const* DummyProgram::InitViewportInfo (
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

VkPipelineVertexInputStateCreateInfo const* DummyProgram::InitVertexInputInfo (
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
