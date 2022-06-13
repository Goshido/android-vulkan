#include <pbr/light_volume_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class LightVolumeDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

    private:
        std::atomic_size_t          _references = 0U;

    public:
        LightVolumeDescriptorSetLayoutImpl () = default;

        LightVolumeDescriptorSetLayoutImpl ( LightVolumeDescriptorSetLayoutImpl const & ) = delete;
        LightVolumeDescriptorSetLayoutImpl& operator = ( LightVolumeDescriptorSetLayoutImpl const & ) = delete;

        LightVolumeDescriptorSetLayoutImpl ( LightVolumeDescriptorSetLayoutImpl && ) = delete;
        LightVolumeDescriptorSetLayoutImpl& operator = ( LightVolumeDescriptorSetLayoutImpl && ) = delete;

        ~LightVolumeDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

void LightVolumeDescriptorSetLayoutImpl::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::LightVolumeDescriptorSetLayoutImpl::_layout" )
}

bool LightVolumeDescriptorSetLayoutImpl::Init ( VkDevice device ) noexcept
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
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
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
        "pbr::LightVolumeDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::LightVolumeDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static LightVolumeDescriptorSetLayoutImpl g_lightVolumeDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void LightVolumeDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_lightVolumeDescriptorSetLayout.Destroy ( device );
}

bool LightVolumeDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_lightVolumeDescriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout LightVolumeDescriptorSetLayout::GetLayout () const noexcept
{
    return g_lightVolumeDescriptorSetLayout._layout;
}

} // namespace pbr
