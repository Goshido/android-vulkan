#include <pbr/spd.inc>
#include <pbr/spd_12_mips_program.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/spd_12_mips.cs.spv";

} // end of anonymous namespace

SPD12MipsProgram::SPD12MipsProgram () noexcept:
    ComputeProgram ( "pbr::SPD12MipsProgram" )
{
    // NOTHING
}

bool SPD12MipsProgram::Init ( android_vulkan::Renderer &renderer, SpecializationData specializationData ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    VkSpecializationInfo specInfo {};

    VkComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;

    bool result = InitShaderInfo ( renderer, specializationData, &specInfo, pipelineInfo.stage ) &&
        InitLayout ( device, pipelineInfo.layout );

    if ( !result )
        return false;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::SPD12MipsProgram::Init",
        "Can't create pipeline"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE ( "pbr::SPD12MipsProgram::_pipeline" )
    DestroyShaderModule ( device );
    return true;
}

void SPD12MipsProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_REGISTER_PIPELINE_LAYOUT ( "pbr::SPD12MipsProgram::_pipelineLayout" )
    }

    _layout.Destroy ( device );
    DestroyShaderModule ( device );

    if ( _pipeline == VK_NULL_HANDLE )
        return;

    vkDestroyPipeline ( device, _pipeline, nullptr );
    _pipeline = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE ( "pbr::SPD12MipsProgram::_pipeline" )
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

void SPD12MipsProgram::GetMetaInfo ( VkExtent3D &dispatch,
    SpecializationInfo &specializationInfo,
    VkExtent2D const &imageResolution
) noexcept
{
    constexpr uint32_t multipleOf64 = 6U;

    dispatch =
    {
        .width = imageResolution.width >> multipleOf64,
        .height = imageResolution.height >> multipleOf64,
        .depth = 1U
    };

    specializationInfo._workgroupCount = dispatch.width * dispatch.height;
}

void SPD12MipsProgram::DestroyShaderModule ( VkDevice device ) noexcept
{
    if ( _computeShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _computeShader, nullptr );
    _computeShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "pbr::SPD12MipsProgram::_computeShader" )
}

bool SPD12MipsProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_layout.Init ( device ) )
        return false;

    VkDescriptorSetLayout dsLayout = _layout.GetLayout ();

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &dsLayout,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::SPD12MipsProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "pbr::SPD12MipsProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

bool SPD12MipsProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    SpecializationData specializationData,
    VkSpecializationInfo* specializationInfo,
    VkPipelineShaderStageCreateInfo &targetInfo
) noexcept
{
    bool const result = renderer.CreateShader ( _computeShader,
        SHADER,
        "Can't create shader (pbr::SPD12MipsProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "pbr::SPD12MipsProgram::_computeShader" )

    constexpr static VkSpecializationMapEntry entry
    {
        .constantID = CONST_WORKGROUP_COUNT,
        .offset = static_cast<uint32_t> ( offsetof ( SpecializationInfo, _workgroupCount ) ),
        .size = sizeof ( SpecializationInfo )
    };

    *specializationInfo =
    {
        .mapEntryCount = 1U,
        .pMapEntries = &entry,
        .dataSize = sizeof ( SpecializationInfo ),
        .pData = specializationData
    };

    targetInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = _computeShader,
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = specializationInfo
    };

    return true;
}

} // namespace pbr
