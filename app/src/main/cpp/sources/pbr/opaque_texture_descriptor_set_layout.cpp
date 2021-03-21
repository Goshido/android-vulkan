#include <pbr/opaque_texture_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class OpaqueTextureDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout;

    private:
        std::atomic<size_t>         _references;

    public:
        OpaqueTextureDescriptorSetLayoutImpl ();

        OpaqueTextureDescriptorSetLayoutImpl ( OpaqueTextureDescriptorSetLayoutImpl const &other ) = delete;
        OpaqueTextureDescriptorSetLayoutImpl& operator = ( OpaqueTextureDescriptorSetLayoutImpl const &other ) = delete;

        OpaqueTextureDescriptorSetLayoutImpl ( OpaqueTextureDescriptorSetLayoutImpl &&other ) = delete;
        OpaqueTextureDescriptorSetLayoutImpl& operator = ( OpaqueTextureDescriptorSetLayoutImpl &&other ) = delete;

        ~OpaqueTextureDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

OpaqueTextureDescriptorSetLayoutImpl::OpaqueTextureDescriptorSetLayoutImpl ():
    _layout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void OpaqueTextureDescriptorSetLayoutImpl::Destroy ( VkDevice device )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueTextureDescriptorSetLayoutImpl::_layout" )
}

bool OpaqueTextureDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    constexpr static VkDescriptorSetLayoutBinding const bindingInfo[] =
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
        },
        {
            .binding = 2U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 3U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 4U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 5U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 6U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 7U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 8U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 9U,
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
        .bindingCount = static_cast<uint32_t> ( std::size ( bindingInfo ) ),
        .pBindings = bindingInfo
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetLayoutInfo, nullptr, &_layout ),
        "OpaqueTextureDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueTextureDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static OpaqueTextureDescriptorSetLayoutImpl g_opaqueTextureDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void OpaqueTextureDescriptorSetLayout::Destroy ( VkDevice device )
{
    g_opaqueTextureDescriptorSetLayout.Destroy ( device );
}

bool OpaqueTextureDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_opaqueTextureDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout OpaqueTextureDescriptorSetLayout::GetLayout () const
{
    return g_opaqueTextureDescriptorSetLayout._layout;
}

} // namespace pbr
