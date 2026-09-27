#include <precompiled_headers.hpp>
#include <file.hpp>
#include <platform/windows/pbr/id_collect.inc>
#include <platform/windows/pbr/id_collect_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <renderer.hpp>


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

    VkComputePipelineCreateInfo const pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .stage = InitShaderInfo ( cs, moduleInfo, nullptr, nullptr ),
        .layout = UniversalPipelineLayout::GetPipelineLayout (),
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
}

VkExtent3D IDCollectProgram::DispatchParams ( int32_t left,
    int32_t right,
    int32_t top,
    int32_t bottom
) noexcept
{
    return
    {
        .width = ( static_cast<uint32_t> ( right - left ) + THREADS_X ) / THREADS_X,
        .height = ( static_cast<uint32_t> ( bottom - top ) + THREADS_Y ) / THREADS_Y,
        .depth = 1U
    };
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
