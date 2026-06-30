#include <precompiled_headers.hpp>
#include <file.hpp>
#include <pbr/exposure.inc>
#include <pbr/exposure_specialization.hpp>
#include <platform/windows/pbr/exposure_program.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <renderer.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/windows/exposure.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ExposureProgram::ExposureProgram () noexcept:
    ComputeProgram ( sizeof ( ExposureProgram::PushConstants ) )
{
    // NOTHING
}

bool ExposureProgram::Init ( VkDevice device, SpecializationData specializationData ) noexcept
{
    VkSpecializationInfo specInfo {};
    VkShaderModuleCreateInfo moduleInfo {};
    std::vector<uint8_t> cs{};

    VkComputePipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .stage = InitShaderInfo ( cs, moduleInfo, specializationData, &specInfo ),
        .layout = UniversalPipelineLayout::GetPipelineLayout (),
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "pbr::ExposureProgram::Init",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Exposure" )
    return true;
}

void ExposureProgram::Destroy ( VkDevice device ) noexcept
{
    ComputeProgram::Destroy ( device );
}

VkPipelineShaderStageCreateInfo ExposureProgram::InitShaderInfo ( std::vector<uint8_t> &cs,
    VkShaderModuleCreateInfo &moduleInfo,
    SpecializationData specializationData,
    VkSpecializationInfo* specializationInfo
) noexcept
{
    android_vulkan::File csFile ( SHADER );

    if ( !csFile.LoadContent () ) [[unlikely]]
        return {};

    cs = std::move ( csFile.GetContent () );

    constexpr size_t w = offsetof ( VkExtent2D, width );
    constexpr size_t h = offsetof ( VkExtent2D, height );

    constexpr static VkSpecializationMapEntry const entries[] =
    {
        {
            .constantID = CONST_LAST_WORKGROUP_INDEX,
            .offset = static_cast<uint32_t> ( offsetof ( ExposureSpecialization, _lastWorkgroupIndex ) ),
            .size = sizeof ( ExposureSpecialization::_lastWorkgroupIndex )
        },
        {
            .constantID = CONST_MIP_5_W,
            .offset = static_cast<uint32_t> ( offsetof ( ExposureSpecialization, _mip5Resolution ) + w ),
            .size = sizeof ( VkExtent2D::width )
        },
        {
            .constantID = CONST_MIP_5_H,
            .offset = static_cast<uint32_t> ( offsetof ( ExposureSpecialization, _mip5Resolution ) + h ),
            .size = sizeof ( VkExtent2D::height )
        },
        {
            .constantID = CONST_NORMALIZE_W,
            .offset = static_cast<uint32_t> ( offsetof ( ExposureSpecialization, _normalizeW ) ),
            .size = sizeof ( ExposureSpecialization::_normalizeW )
        },
        {
            .constantID = CONST_NORMALIZE_H,
            .offset = static_cast<uint32_t> ( offsetof ( ExposureSpecialization, _normalizeH ) ),
            .size = sizeof ( ExposureSpecialization::_normalizeH )
        }
    };

    moduleInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .codeSize = cs.size (),
        .pCode = reinterpret_cast<uint32_t const*> ( cs.data () )
    };

    *specializationInfo =
    {
        .mapEntryCount = static_cast<uint32_t> ( std::size ( entries ) ),
        .pMapEntries = entries,
        .dataSize = sizeof ( ExposureSpecialization ),
        .pData = specializationData
    };

    return
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &moduleInfo,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = VK_NULL_HANDLE,
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = specializationInfo
    };
}

} // namespace pbr
