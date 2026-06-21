#include <precompiled_headers.hpp>
#include <file.hpp>
#include <platform/windows/pbr/id_collect.inc>
#include <platform/windows/pbr/id_collect_program.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/windows/id_collect.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

IDCollectProgram::IDCollectProgram () noexcept:
    ComputeProgram ( sizeof ( IDCollectProgram::PushConstants ) )
{
    // NOTHING
}

bool IDCollectProgram::Init ( VkDevice device, SpecializationData /*specializationData*/ ) noexcept
{
    VkShaderModuleCreateInfo moduleInfo {};
    std::vector<uint8_t> cs{};

    VkComputePipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .stage = InitShaderInfo ( cs, moduleInfo, nullptr, nullptr ),
        .layout = InitLayout ( device ),
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::IDCollectProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "ID collect" )
    return true;
}

void IDCollectProgram::Destroy ( VkDevice device ) noexcept
{
    ComputeProgram::Destroy ( device );
    _layout.Destroy ( device );
}

VkExtent3D IDCollectProgram::DispatchParams ( VkExtent2D const &resolution ) noexcept
{
    return
    {
        .width = ( resolution.width + THREADS_X - 1U ) / THREADS_X,
        .height = ( resolution.height + THREADS_Y - 1U ) / THREADS_Y,
        .depth = 1U
    };
}

VkPipelineLayout IDCollectProgram::InitLayout ( VkDevice device ) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return VK_NULL_HANDLE;

    constexpr VkPushConstantRange pushConstantRange
    {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0U,
        .size = static_cast<uint32_t> ( sizeof ( IDCollectProgram::PushConstants ) )
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
        "pbr::IDCollectProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return VK_NULL_HANDLE;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "ID collect" )
    return _pipelineLayout;
}

VkPipelineShaderStageCreateInfo IDCollectProgram::InitShaderInfo ( std::vector<uint8_t> &cs,
    VkShaderModuleCreateInfo &moduleInfo,
    SpecializationData /*specializationData*/,
    VkSpecializationInfo* /*specializationInfo*/
) noexcept
{
    android_vulkan::File csFile ( SHADER );

    if ( !csFile.LoadContent () ) [[unlikely]]
        return {};

    cs = std::move ( csFile.GetContent () );

    moduleInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .codeSize = cs.size (),
        .pCode = reinterpret_cast<uint32_t const*> ( cs.data () )
    };

    return
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &moduleInfo,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = VK_NULL_HANDLE,
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };
}

} // namespace pbr
