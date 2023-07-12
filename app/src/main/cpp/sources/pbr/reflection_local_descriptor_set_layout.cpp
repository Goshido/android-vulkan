#include <pbr/reflection_local_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class ReflectionLocalDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

    private:
        std::atomic_size_t          _references = 0U;

    public:
        ReflectionLocalDescriptorSetLayoutImpl () = default;
    
        ReflectionLocalDescriptorSetLayoutImpl ( ReflectionLocalDescriptorSetLayoutImpl const & ) = delete;
        ReflectionLocalDescriptorSetLayoutImpl &operator = ( ReflectionLocalDescriptorSetLayoutImpl const & ) = delete;
    
        ReflectionLocalDescriptorSetLayoutImpl ( ReflectionLocalDescriptorSetLayoutImpl && ) = delete;
        ReflectionLocalDescriptorSetLayoutImpl &operator = ( ReflectionLocalDescriptorSetLayoutImpl && ) = delete;
    
        ~ReflectionLocalDescriptorSetLayoutImpl () = default;
    
        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

void ReflectionLocalDescriptorSetLayoutImpl::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::ReflectionLocalDescriptorSetLayoutImpl::_layout" )
}

bool ReflectionLocalDescriptorSetLayoutImpl::Init ( VkDevice device ) noexcept
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
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    constexpr VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindings ) ),
        .pBindings = bindings
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_layout ),
        "pbr::ReflectionLocalDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::ReflectionLocalDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static ReflectionLocalDescriptorSetLayoutImpl g_reflectionLocalDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void ReflectionLocalDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_reflectionLocalDescriptorSetLayout.Destroy ( device );
}

bool ReflectionLocalDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_reflectionLocalDescriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout ReflectionLocalDescriptorSetLayout::GetLayout () const noexcept
{
    return g_reflectionLocalDescriptorSetLayout._layout;
}

} // namespace pbr
