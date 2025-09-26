#include <precompiled_headers.hpp>
#include <platform/windows/pbr/resource_heap_descriptor_set_layout_ext.hpp>
#include <platform/windows/pbr/samplers.inc>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

// See https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#resourcedescriptorheaps-samplerdescriptorheaps
constexpr uint32_t RESOURCE_BINDING = 0U;

//----------------------------------------------------------------------------------------------------------------------

class DescriptorSetLayout final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

    private:
        std::atomic_size_t          _references = 0U;

    public:
        DescriptorSetLayout () = default;

        DescriptorSetLayout ( DescriptorSetLayout const & ) = delete;
        DescriptorSetLayout &operator = ( DescriptorSetLayout const & ) = delete;

        DescriptorSetLayout ( DescriptorSetLayout && ) = delete;
        DescriptorSetLayout &operator = ( DescriptorSetLayout && ) = delete;

        ~DescriptorSetLayout () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device, uint32_t resourceCapacity ) noexcept;
};

void DescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    if ( _references > 0U && --_references == 0U )
    {
        vkDestroyDescriptorSetLayout ( device, std::exchange ( _layout, VK_NULL_HANDLE ), nullptr );
    }
}

bool DescriptorSetLayout::Init ( VkDevice device, uint32_t resourceCapacity ) noexcept
{
    if ( ++_references != 1U ) [[likely]]
        return true;

    constexpr static VkDescriptorBindingFlags const bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    constexpr static VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = 1U,
        .pBindingFlags = &bindingFlags
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

    VkDescriptorSetLayoutBinding const bindings
    {
        .binding = RESOURCE_BINDING,
        .descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
        .descriptorCount = resourceCapacity,
        .stageFlags = stages,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &mutableDescriptorTypeInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        .bindingCount = 1U,
        .pBindings = &bindings
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &info, nullptr, &_layout ),
        "pbr::ResourceHeapDescriptorSetLayoutEXT::Init",
        "Can't create descriptor set layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Resource heap ext" )
    return true;
}

DescriptorSetLayout g_descriptorSetLayout {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

uint32_t ResourceHeapDescriptorSetLayoutEXT::_resourceCapacity = 0U;

void ResourceHeapDescriptorSetLayoutEXT::Destroy ( VkDevice device ) noexcept
{
    if ( _init )
    {
        g_descriptorSetLayout.Destroy ( device );
        _init = false;
    }
}

bool ResourceHeapDescriptorSetLayoutEXT::Init ( VkDevice device ) noexcept
{
    _init = true;
    return g_descriptorSetLayout.Init ( device, _resourceCapacity );
}

VkDescriptorSetLayout &ResourceHeapDescriptorSetLayoutEXT::GetLayout () const noexcept
{
    return g_descriptorSetLayout._layout;
}

void ResourceHeapDescriptorSetLayoutEXT::SetResourceCapacity ( uint32_t capacity ) noexcept
{
    _resourceCapacity = capacity;
}

} // namespace pbr
