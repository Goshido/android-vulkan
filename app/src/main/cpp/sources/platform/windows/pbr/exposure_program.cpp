#include <precompiled_headers.hpp>
#include <file.hpp>
#include <pbr/exposure.inc>
#include <pbr/exposure_specialization.hpp>
#include <platform/windows/pbr/exposure_program.hpp>


// FUCK - remove namespace
namespace pbr::windows {

namespace {

constexpr char const* SHADER = "shaders/windows/exposure.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ExposureProgram::ExposureProgram () noexcept:
    ComputeProgram ( "pbr::ExposureProgram", sizeof ( ExposureProgram::PushConstants ) )
{
    // NOTHING
}

bool ExposureProgram::Init ( VkDevice device, SpecializationData specializationData ) noexcept
{
    VkSpecializationInfo specInfo {};
    VkShaderModuleCreateInfo moduleInfo {};
    std::vector<uint8_t> cs{};

    VkComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    bool const result = InitShaderInfo ( cs, moduleInfo, specializationData, &specInfo, pipelineInfo.stage ) &&
        InitLayout ( device, pipelineInfo.layout ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),

            // FUCK - remove namespace
            "pbr::windows::ExposureProgram::Init",

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
    _layoutExt.Destroy ( device );
    _layout.Destroy ( device );
}

bool ExposureProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_layout.Init ( device ) || !_layoutExt.Init ( device ) ) [[unlikely]]
        return false;

    constexpr VkPushConstantRange pushConstantRange
    {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0U,
        .size = static_cast<uint32_t> ( sizeof ( ExposureProgram::PushConstants ) )
    };

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        //.pSetLayouts = &_layout.GetLayout (),
        .pSetLayouts = &_layoutExt.GetLayout (),
        .pushConstantRangeCount = 1U,
        .pPushConstantRanges = &pushConstantRange
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),

        // FUCK - remove namespace
        "pbr::windows::ExposureProgram::InitLayout",

        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Exposure" )

    layout = _pipelineLayout;
    return true;
}

bool ExposureProgram::InitShaderInfo ( std::vector<uint8_t> &cs,
    VkShaderModuleCreateInfo &moduleInfo,
    SpecializationData specializationData,
    VkSpecializationInfo* specializationInfo,
    VkPipelineShaderStageCreateInfo &targetInfo
) noexcept
{
    android_vulkan::File csFile ( SHADER );

    if ( !csFile.LoadContent () ) [[unlikely]]
        return false;

    cs = std::move ( csFile.GetContent () );

    constexpr size_t w = offsetof ( VkExtent2D, width );
    constexpr size_t h = offsetof ( VkExtent2D, height );

    constexpr static VkSpecializationMapEntry const entries[] =
    {
        {
            .constantID = CONST_WORKGROUP_COUNT,
            .offset = static_cast<uint32_t> ( offsetof ( ExposureSpecialization, _workgroupCount ) ),
            .size = sizeof ( ExposureSpecialization::_workgroupCount )
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

    targetInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &moduleInfo,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = VK_NULL_HANDLE,
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = specializationInfo
    };

    return true;
}

} // namespace pbr::windows
