#include <pbr/geometry_pass_texture_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class GeometryPassTextureDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

    private:
        std::atomic<size_t>         _references = 0U;

    public:
        GeometryPassTextureDescriptorSetLayoutImpl () = default;

        GeometryPassTextureDescriptorSetLayoutImpl ( GeometryPassTextureDescriptorSetLayoutImpl const & ) = delete;

        GeometryPassTextureDescriptorSetLayoutImpl& operator = (
            GeometryPassTextureDescriptorSetLayoutImpl const &
        ) = delete;

        GeometryPassTextureDescriptorSetLayoutImpl ( GeometryPassTextureDescriptorSetLayoutImpl && ) = delete;

        GeometryPassTextureDescriptorSetLayoutImpl& operator = (
            GeometryPassTextureDescriptorSetLayoutImpl &&
        ) = delete;

        ~GeometryPassTextureDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
};

void GeometryPassTextureDescriptorSetLayoutImpl::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "GeometryPassTextureDescriptorSetLayoutImpl::_layout" )
}

bool GeometryPassTextureDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer ) noexcept
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

    constexpr VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindingInfo ) ),
        .pBindings = bindingInfo
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetLayoutInfo, nullptr, &_layout ),
        "pbr::GeometryPassTextureDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "GeometryPassTextureDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static GeometryPassTextureDescriptorSetLayoutImpl g_opaqueTextureDescriptorSetLayout {};

//----------------------------------------------------------------------------------------------------------------------

void GeometryPassTextureDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_opaqueTextureDescriptorSetLayout.Destroy ( device );
}

bool GeometryPassTextureDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    return g_opaqueTextureDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout GeometryPassTextureDescriptorSetLayout::GetLayout () const noexcept
{
    return g_opaqueTextureDescriptorSetLayout._layout;
}

} // namespace pbr
