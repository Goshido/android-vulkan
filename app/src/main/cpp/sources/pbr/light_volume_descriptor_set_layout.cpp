#include <pbr/light_volume_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class LightVolumeDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout;

    private:
        std::atomic_size_t          _references;

    public:
        LightVolumeDescriptorSetLayoutImpl () noexcept;

        LightVolumeDescriptorSetLayoutImpl ( LightVolumeDescriptorSetLayoutImpl const & ) = delete;
        LightVolumeDescriptorSetLayoutImpl& operator = ( LightVolumeDescriptorSetLayoutImpl const & ) = delete;

        LightVolumeDescriptorSetLayoutImpl ( LightVolumeDescriptorSetLayoutImpl && ) = delete;
        LightVolumeDescriptorSetLayoutImpl& operator = ( LightVolumeDescriptorSetLayoutImpl && ) = delete;

        ~LightVolumeDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

LightVolumeDescriptorSetLayoutImpl::LightVolumeDescriptorSetLayoutImpl () noexcept:
    _layout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void LightVolumeDescriptorSetLayoutImpl::Destroy ( VkDevice device )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "LightVolumeDescriptorSetLayoutImpl::_layout" )
}

bool LightVolumeDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
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
        "LightVolumeDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "LightVolumeDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static LightVolumeDescriptorSetLayoutImpl g_lightVolumeDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void LightVolumeDescriptorSetLayout::Destroy ( VkDevice device )
{
    g_lightVolumeDescriptorSetLayout.Destroy ( device );
}

bool LightVolumeDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_lightVolumeDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout LightVolumeDescriptorSetLayout::GetLayout () const
{
    return g_lightVolumeDescriptorSetLayout._layout;
}

} // namespace pbr
