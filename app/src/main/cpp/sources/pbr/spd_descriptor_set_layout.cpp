#include <pbr/spd.inc>
#include <pbr/spd_descriptor_set_layout.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

SPDDescriptorSetLayout::SPDDescriptorSetLayout ( uint32_t relaxedMipViews ) noexcept:
    _relaxedMipViews ( relaxedMipViews )
{
    // NOTHING
}

void SPDDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;

    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT (
        "pbr::SPD" + std::to_string ( _relaxedMipViews + 3U ) + "MipsDescriptorSetLayout::_layout"
    )
}

bool SPDDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    VkDescriptorSetLayoutBinding const bindings[] =
    {
        {
            .binding = BIND_HDR_IMAGE,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_SYNC_MIP_5,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_MIPS,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = _relaxedMipViews,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_GLOBAL_ATOMIC,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_BRIGHTNESS,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    VkDescriptorSetLayoutCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindings ) ),
        .pBindings = bindings
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &info, nullptr, &_layout ),
        "pbr::SPD12MipsDescriptorSetLayout::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT (
        "pbr::SPD" + std::to_string ( _relaxedMipViews + 3U ) + "MipsDescriptorSetLayout::_layout"
    )

    ++_references;
    return true;
}

} // namespace pbr
