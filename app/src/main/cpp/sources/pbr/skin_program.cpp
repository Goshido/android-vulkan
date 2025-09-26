#include <precompiled_headers.hpp>
#include <pbr/skin.inc>
#include <pbr/skin_program.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/skin.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SkinProgram::SkinProgram () noexcept:
    ComputeProgram ( "pbr::SkinProgram", sizeof ( SkinProgram::PushConstants ) )
{
    // NOTHING
}

bool SkinProgram::Init ( android_vulkan::Renderer const &renderer,
    SpecializationData /*specializationData*/
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    VkComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;

    bool result = InitShaderInfo ( renderer, nullptr, nullptr, pipelineInfo.stage ) &&
        InitLayout ( device, pipelineInfo.layout );

    if ( !result ) [[unlikely]]
        return false;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::SkinProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Skin" )
    DestroyShaderModule ( device );
    return true;
}

void SkinProgram::Destroy ( VkDevice device ) noexcept
{
    ComputeProgram::Destroy ( device );
    _layout.Destroy ( device );
}

void SkinProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        _pipelineLayout,
        0U,
        1U,
        &set,
        0U,
        nullptr
    );
}

bool SkinProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    constexpr VkPushConstantRange pushConstantRange
    {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0U,
        .size = static_cast<uint32_t> ( sizeof ( SkinProgram::PushConstants ) )
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
        "pbr::SkinProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Skin" )
    layout = _pipelineLayout;
    return true;
}

bool SkinProgram::InitShaderInfo ( android_vulkan::Renderer const &renderer,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/,
    VkPipelineShaderStageCreateInfo &targetInfo
) noexcept
{
    bool const result = renderer.CreateShader ( _computeShader,
        SHADER,
        "Can't create shader (pbr::SkinProgram)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _computeShader, VK_OBJECT_TYPE_SHADER_MODULE, SHADER )

    targetInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = _computeShader,
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };

    return true;
}

} // namespace pbr
