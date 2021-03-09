#include <pbr/reflection_global_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class ReflectionGlobalDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout;

    private:
        std::atomic_size_t          _references;

    public:
        ReflectionGlobalDescriptorSetLayoutImpl () noexcept;

        ReflectionGlobalDescriptorSetLayoutImpl ( ReflectionGlobalDescriptorSetLayoutImpl const & ) = delete;
        ReflectionGlobalDescriptorSetLayoutImpl& operator = ( ReflectionGlobalDescriptorSetLayoutImpl const & ) = delete;

        ReflectionGlobalDescriptorSetLayoutImpl ( ReflectionGlobalDescriptorSetLayoutImpl && ) = delete;
        ReflectionGlobalDescriptorSetLayoutImpl& operator = ( ReflectionGlobalDescriptorSetLayoutImpl && ) = delete;

        ~ReflectionGlobalDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

ReflectionGlobalDescriptorSetLayoutImpl::ReflectionGlobalDescriptorSetLayoutImpl () noexcept:
    _layout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void ReflectionGlobalDescriptorSetLayoutImpl::Destroy ( VkDevice device )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "ReflectionGlobalDescriptorSetLayoutImpl::_layout" )
}

bool ReflectionGlobalDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    constexpr static VkDescriptorSetLayoutBinding const bindings[] =
    {
        {
            .binding = 0U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    constexpr VkDescriptorSetLayoutCreateInfo const descriptorSetLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindings ) ),
        .pBindings = bindings
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetLayoutInfo, nullptr, &_layout ),
        "ReflectionGlobalDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "ReflectionGlobalDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static ReflectionGlobalDescriptorSetLayoutImpl g_reflectionGlobalDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void ReflectionGlobalDescriptorSetLayout::Destroy ( VkDevice device )
{
    g_reflectionGlobalDescriptorSetLayout.Destroy ( device );
}

bool ReflectionGlobalDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_reflectionGlobalDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout ReflectionGlobalDescriptorSetLayout::GetLayout () const
{
    return g_reflectionGlobalDescriptorSetLayout._layout;
}

} // namespace pbr
