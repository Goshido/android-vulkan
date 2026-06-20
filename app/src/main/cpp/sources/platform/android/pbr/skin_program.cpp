#include <precompiled_headers.hpp>
#include <platform/android/pbr/skin.inc>
#include <platform/android/pbr/skin_program.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/skin.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SkinProgram::SkinProgram () noexcept:
    ComputeProgram ( sizeof ( SkinProgram::PushConstants ) )
{
    // NOTHING
}

bool SkinProgram::Init ( android_vulkan::Renderer const &renderer,
    SpecializationData /*specializationData*/
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    VkComputePipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = InitShaderInfo ( renderer, nullptr, nullptr ),
        .layout = InitLayout ( device ),
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
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

VkPipelineLayout SkinProgram::InitLayout ( VkDevice device ) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return VK_NULL_HANDLE;

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
        return VK_NULL_HANDLE;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Skin" )
    return _pipelineLayout;
}

VkPipelineShaderStageCreateInfo SkinProgram::InitShaderInfo ( android_vulkan::Renderer const &renderer,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/
) noexcept
{
    bool const result = renderer.CreateShader ( _computeShader, SHADER, "Can't create shader (pbr::SkinProgram)" );

    if ( !result ) [[unlikely]]
        return {};

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _computeShader, VK_OBJECT_TYPE_SHADER_MODULE, SHADER )

    return
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = _computeShader,
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };
}

} // namespace pbr
