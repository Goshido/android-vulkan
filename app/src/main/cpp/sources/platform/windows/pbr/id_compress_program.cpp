#include <precompiled_headers.hpp>
#include <file.hpp>
#include <platform/windows/pbr/id_compress.inc>
#include <platform/windows/pbr/id_compress_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <renderer.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/windows/id_compress.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

IDCompressProgram::IDCompressProgram () noexcept:
    ComputeProgram ( sizeof ( IDCompressProgram::PushConstants ) )
{
    // NOTHING
}

bool IDCompressProgram::Init ( VkDevice device, SpecializationData /*specializationData*/ ) noexcept
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
        "pbr::IDCompressProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "ID compress" )
    return true;
}

void IDCompressProgram::Destroy ( VkDevice device ) noexcept
{
    ComputeProgram::Destroy ( device );
}

VkExtent3D IDCompressProgram::DispatchParams ( uint32_t capacity ) noexcept
{
    constexpr uint32_t pixelsPerWorkgroup = THREADS * WINDOW;

    return
    {
        .width = ( capacity + pixelsPerWorkgroup - 1U ) / pixelsPerWorkgroup,
        .height = 1U,
        .depth = 1U
    };
}

VkPipelineShaderStageCreateInfo IDCompressProgram::InitShaderInfo ( std::vector<uint8_t> &cs,
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
