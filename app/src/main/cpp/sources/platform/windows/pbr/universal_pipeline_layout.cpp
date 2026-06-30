#include <precompiled_headers.hpp>
#include <platform/windows/pbr/resource_heap.inc>
#include <platform/windows/pbr/push_constant_range.inc>
#include <platform/windows/pbr/samplers.inc>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <renderer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

class PipelineLayout final
{
    public:
        VkDescriptorSetLayout       _descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout            _pipelineLayout = VK_NULL_HANDLE;

    public:
        PipelineLayout () = default;

        PipelineLayout ( PipelineLayout const & ) = delete;
        PipelineLayout &operator = ( PipelineLayout const & ) = delete;

        PipelineLayout ( PipelineLayout && ) = delete;
        PipelineLayout &operator = ( PipelineLayout && ) = delete;

        ~PipelineLayout () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device, uint32_t resourceCapacity ) noexcept;
};

void PipelineLayout::Destroy ( VkDevice device ) noexcept
{
    vkDestroyPipelineLayout ( device, std::exchange ( _pipelineLayout, VK_NULL_HANDLE ), nullptr );
    vkDestroyDescriptorSetLayout ( device, std::exchange ( _descriptorSetLayout, VK_NULL_HANDLE ), nullptr );
}

bool PipelineLayout::Init ( VkDevice device, uint32_t resourceCapacity ) noexcept
{
    constexpr static VkDescriptorBindingFlags const bindingFlags[] =
    {
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        0U
    };

    constexpr static VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindingFlags ) ),
        .pBindingFlags = bindingFlags
    };

    constexpr static VkDescriptorType const descriptorTypes[] =
    {
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    };

    constexpr static VkMutableDescriptorTypeListEXT mutableListInfo
    {
        .descriptorTypeCount = static_cast<uint32_t> ( std::size ( descriptorTypes ) ),
        .pDescriptorTypes = descriptorTypes
    };

    constexpr static VkMutableDescriptorTypeCreateInfoEXT mutableDescriptorTypeInfo
    {
        .sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT,
        .pNext = &bindingFlagsInfo,
        .mutableDescriptorTypeListCount = 1U,
        .pMutableDescriptorTypeLists = &mutableListInfo
    };

    constexpr VkShaderStageFlags stages = AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) |
        AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT ) |
        AV_VK_FLAG ( VK_SHADER_STAGE_COMPUTE_BIT );

    VkDescriptorSetLayoutBinding const bindings[] =
    {
        {
            .binding = BIND_RESOURCES,
            .descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
            .descriptorCount = resourceCapacity,
            .stageFlags = stages,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_SAMPLERS,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = TOTAL_SAMPLERS,
            .stageFlags = stages,
            .pImmutableSamplers = nullptr
        }
    };

    VkDescriptorSetLayoutCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &mutableDescriptorTypeInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindings ) ),
        .pBindings = bindings
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &info, nullptr, &_descriptorSetLayout ),
        "pbr::UniversalPipelineLayout::Init",
        "Can't create descriptor set layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Universal" )

    constexpr VkPushConstantRange pushConstantRange
    {
        .stageFlags = stages,
        .offset = 0U,
        .size = PUSH_CONSTANT_RANGE
    };

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &_descriptorSetLayout,
        .pushConstantRangeCount = 1U,
        .pPushConstantRanges = &pushConstantRange
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::UniversalPipelineLayout::Init",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Universal" )
    return true;
}

PipelineLayout g_pipelineLayout {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

uint32_t UniversalPipelineLayout::_resourceCapacity = 0U;

void UniversalPipelineLayout::Destroy ( VkDevice device ) noexcept
{
    g_pipelineLayout.Destroy ( device );
}

bool UniversalPipelineLayout::Init ( VkDevice device ) noexcept
{
    return g_pipelineLayout.Init ( device, _resourceCapacity );
}

VkDescriptorSetLayout &UniversalPipelineLayout::GetDescriptorSetLayout () noexcept
{
    return g_pipelineLayout._descriptorSetLayout;
}

VkPipelineLayout &UniversalPipelineLayout::GetPipelineLayout () noexcept
{
    return g_pipelineLayout._pipelineLayout;
}

void UniversalPipelineLayout::SetResourceCapacity ( uint32_t capacity ) noexcept
{
    _resourceCapacity = capacity;
}

} // namespace pbr
