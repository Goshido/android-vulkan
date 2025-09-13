#include <precompiled_headers.hpp>
#include <pbr/exposure.inc>
#include <pbr/exposure_specialization.hpp>
#include <platform/android/pbr/exposure_program.hpp>


// FUCK - remove namespace
namespace pbr::android {

namespace {

constexpr char const* SHADER = "shaders/android/exposure.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ExposureProgram::ExposureProgram () noexcept:
    ComputeProgram ( "pbr::ExposureProgram", sizeof ( ExposureProgram::PushConstants ) )
{
    // NOTHING
}

bool ExposureProgram::Init ( android_vulkan::Renderer const &renderer,
    SpecializationData specializationData
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    VkSpecializationInfo specInfo {};

    VkComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    bool const result = InitShaderInfo ( renderer, specializationData, &specInfo, pipelineInfo.stage ) &&
        InitLayout ( device, pipelineInfo.layout ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkCreateComputePipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),

            // FUCK - remove namespace
            "pbr::android::ExposureProgram::Init",

            "Can't create pipeline"
        );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Exposure" )
    DestroyShaderModule ( device );
    return true;
}

void ExposureProgram::Destroy ( VkDevice device ) noexcept
{
    ComputeProgram::Destroy ( device );
    _layout.Destroy ( device );
}

void ExposureProgram::SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept
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

bool ExposureProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
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
        .pSetLayouts = &_layout.GetLayout (),
        .pushConstantRangeCount = 1U,
        .pPushConstantRanges = &pushConstantRange
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),

        // FUCK - remove namespace
        "pbr::android::ExposureProgram::InitLayout",

        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Exposure" )

    layout = _pipelineLayout;
    return true;
}

bool ExposureProgram::InitShaderInfo ( android_vulkan::Renderer const &renderer,
    SpecializationData specializationData,
    VkSpecializationInfo* specializationInfo,
    VkPipelineShaderStageCreateInfo &targetInfo
) noexcept
{
    bool const result = renderer.CreateShader ( _computeShader,
        SHADER,

        // FUCK - remove namespace
        "Can't create shader (pbr::android::ExposureProgram)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _computeShader, VK_OBJECT_TYPE_SHADER_MODULE, SHADER )

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
        .pNext = nullptr,
        .flags = 0U,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = _computeShader,
        .pName = COMPUTE_SHADER_ENTRY_POINT,
        .pSpecializationInfo = specializationInfo
    };

    return true;
}

} // namespace pbr::android
