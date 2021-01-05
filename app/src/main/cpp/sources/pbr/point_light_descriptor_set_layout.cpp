#include <pbr/point_light_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class PointLightDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _descriptorSetLayout;

    private:
        std::atomic_size_t          _references;

    public:
        PointLightDescriptorSetLayoutImpl () noexcept;

        PointLightDescriptorSetLayoutImpl ( PointLightDescriptorSetLayoutImpl const & ) = delete;
        PointLightDescriptorSetLayoutImpl& operator = ( PointLightDescriptorSetLayoutImpl const & ) = delete;

        PointLightDescriptorSetLayoutImpl ( PointLightDescriptorSetLayoutImpl && ) = delete;
        PointLightDescriptorSetLayoutImpl& operator = ( PointLightDescriptorSetLayoutImpl && ) = delete;

        ~PointLightDescriptorSetLayoutImpl () = default;

        void Destroy ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

PointLightDescriptorSetLayoutImpl::PointLightDescriptorSetLayoutImpl () noexcept:
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void PointLightDescriptorSetLayoutImpl::Destroy ( android_vulkan::Renderer &renderer )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( renderer.GetDevice (), _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "PointLightDescriptorSetLayoutImpl::_descriptorSetLayout" )
}

bool PointLightDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
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
        },
        {
            .binding = 2U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
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
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
        "PointLightDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "PointLightDescriptorSetLayoutImpl::_descriptorSetLayout" )

    ++_references;
    return true;
}

static PointLightDescriptorSetLayoutImpl g_pointLightDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void PointLightDescriptorSetLayout::Destroy ( android_vulkan::Renderer &renderer )
{
    g_pointLightDescriptorSetLayout.Destroy ( renderer );
}

bool PointLightDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_pointLightDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout PointLightDescriptorSetLayout::GetLayout () const
{
    return g_pointLightDescriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
