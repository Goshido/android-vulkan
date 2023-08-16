#include <pbr/spd_12_mips_program.hpp>


namespace pbr {

namespace {

[[maybe_unused]] constexpr char const* SHADER = "shaders/spd_12_mips.cs.spv";

} // end of anonymous namespace

SPD12MipsProgram::SPD12MipsProgram () noexcept:
    ComputeProgram ( "pbr::SPD12MipsProgram" )
{
    // NOTHING
}

bool SPD12MipsProgram::Init ( android_vulkan::Renderer& renderer ) noexcept
{
    VkComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;

    VkDevice device = renderer.GetDevice ();

    if ( !InitShaderInfo ( renderer, pipelineInfo.stage ) || !InitLayout ( device, pipelineInfo.layout ) )
        return false;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::SPD12MipsProgram::Init",
        "Can't create pipeline"
    );

    if ( result )
        return false;

    // TODO destroy shader module
    return true;
}

void SPD12MipsProgram::Destroy ( VkDevice /*device*/ ) noexcept
{
    // TODO
}

void SPD12MipsProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept
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

bool SPD12MipsProgram::InitLayout ( VkDevice /*device*/, VkPipelineLayout &/*layout*/ ) noexcept
{
    // TODO
    return true;
}

bool SPD12MipsProgram::InitShaderInfo ( android_vulkan::Renderer &/*renderer*/,
    VkPipelineShaderStageCreateInfo &targetInfo
) noexcept
{
    // TODO

    targetInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = VK_NULL_HANDLE,                   // TODO!!!!
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr              // TODO!!!
    };

    return true;
}

} // namespace pbr
