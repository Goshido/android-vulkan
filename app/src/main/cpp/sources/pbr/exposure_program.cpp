#include <pbr/exposure.inc>
#include <pbr/exposure_program.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/exposure.cs.spv";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ExposureProgram::ExposureProgram () noexcept:
    ComputeProgram ( "pbr::ExposureProgram", sizeof ( ExposureProgram::PushConstants ) )
{
    // NOTHING
}

bool ExposureProgram::Init ( android_vulkan::Renderer &renderer,
    SpecializationData specializationData
) noexcept
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
        "pbr::ExposureProgram::Init",
        "Can't create pipeline"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE ( "pbr::ExposureProgram::_pipeline" )
    DestroyShaderModule ( device );
    return true;
}

void ExposureProgram::Destroy ( VkDevice device ) noexcept
{
    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "pbr::ExposureProgram::_pipelineLayout" )
    }

    _layout.Destroy ( device );
    DestroyShaderModule ( device );

    if ( _pipeline == VK_NULL_HANDLE )
        return;

    vkDestroyPipeline ( device, _pipeline, nullptr );
    _pipeline = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE ( "pbr::ExposureProgram::_pipeline" )
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

void ExposureProgram::GetMetaInfo ( VkExtent3D &dispatch,
    VkExtent2D &mipChainResolution,
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

    // mip 0
    VkExtent2D r
    {
        .width = imageResolution.width >> 1U,
        .height = imageResolution.height >> 1U
    };

    mipChainResolution = r;

    auto const reduce = [] ( uint32_t v ) noexcept -> uint32_t {
        return std::max ( 1U, v >> 1U );
    };

    auto const nextMip = [ & ] ( VkExtent2D resolution ) noexcept -> VkExtent2D {
        return VkExtent2D
        {
            .width = reduce ( resolution.width ),
            .height = reduce ( resolution.height )
        };
    };

    // Making mip 5...
    for ( uint8_t i = 0U; i < 5U; ++i )
        r = nextMip ( r );

    specializationInfo._mip5Resolution = r;
    specializationInfo._normalizeW = 1.0F / static_cast<float> ( r.width );
    specializationInfo._normalizeH = 1.0F / static_cast<float> ( r.height );
}

void ExposureProgram::DestroyShaderModule ( VkDevice device ) noexcept
{
    if ( _computeShader == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _computeShader, nullptr );
    _computeShader = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "pbr::ExposureProgram::_computeShader" )
}

bool ExposureProgram::InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept
{
    if ( !_layout.Init ( device ) )
        return false;

    VkDescriptorSetLayout dsLayout = _layout.GetLayout ();

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
        .pSetLayouts = &dsLayout,
        .pushConstantRangeCount = 1U,
        .pPushConstantRanges = &pushConstantRange
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::ExposureProgram::InitLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "pbr::ExposureProgram::_pipelineLayout" )
    layout = _pipelineLayout;
    return true;
}

bool ExposureProgram::InitShaderInfo ( android_vulkan::Renderer &renderer,
    SpecializationData specializationData,
    VkSpecializationInfo* specializationInfo,
    VkPipelineShaderStageCreateInfo &targetInfo
) noexcept
{
    bool const result = renderer.CreateShader ( _computeShader,
        SHADER,
        "Can't create shader (pbr::ExposureProgram)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "pbr::ExposureProgram::_computeShader" )
    constexpr size_t w = offsetof ( VkExtent2D, width );
    constexpr size_t h = offsetof ( VkExtent2D, height );

    constexpr static VkSpecializationMapEntry const entries[] =
    {
        {
            .constantID = CONST_WORKGROUP_COUNT,
            .offset = static_cast<uint32_t> ( offsetof ( SpecializationInfo, _workgroupCount ) ),
            .size = sizeof ( SpecializationInfo::_workgroupCount )
        },
        {
            .constantID = CONST_MIP_5_W,
            .offset = static_cast<uint32_t> ( offsetof ( SpecializationInfo, _mip5Resolution ) + w ),
            .size = sizeof ( VkExtent2D::width )
        },
        {
            .constantID = CONST_MIP_5_H,
            .offset = static_cast<uint32_t> ( offsetof ( SpecializationInfo, _mip5Resolution ) + h ),
            .size = sizeof ( VkExtent2D::height )
        },
        {
            .constantID = CONST_NORMALIZE_W,
            .offset = static_cast<uint32_t> ( offsetof ( SpecializationInfo, _normalizeW ) ),
            .size = sizeof ( SpecializationInfo::_normalizeW )
        },
        {
            .constantID = CONST_NORMALIZE_H,
            .offset = static_cast<uint32_t> ( offsetof ( SpecializationInfo, _normalizeH ) ),
            .size = sizeof ( SpecializationInfo::_normalizeH )
        }
    };

    *specializationInfo =
    {
        .mapEntryCount = static_cast<uint32_t> ( std::size ( entries ) ),
        .pMapEntries = entries,
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
