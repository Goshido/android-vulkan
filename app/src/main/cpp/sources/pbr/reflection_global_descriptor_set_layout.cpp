#include <precompiled_headers.hpp>
#include <pbr/reflection_global.inc>
#include <pbr/reflection_global_descriptor_set_layout.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

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
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

void DescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
}

bool DescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    constexpr static VkDescriptorSetLayoutBinding const binding
    {
        .binding = BIND_PREFILTER_TEXTURE,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 1U,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    constexpr VkDescriptorSetLayoutCreateInfo const descriptorSetLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = 1U,
        .pBindings = &binding
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_layout ),
        "pbr::ReflectionGlobalDescriptorSetLayout::Init",
        "Can't create descriptor set layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Reflection global" )

    ++_references;
    return true;
}

DescriptorSetLayout g_descriptorSetLayout;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void ReflectionGlobalDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_descriptorSetLayout.Destroy ( device );
}

bool ReflectionGlobalDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout &ReflectionGlobalDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._layout;
}

} // namespace pbr
