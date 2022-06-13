#include <pbr/geometry_pass_sampler_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class GeometryPassSamplerDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

    private:
        std::atomic<size_t>         _references = 0U;

    public:
        GeometryPassSamplerDescriptorSetLayoutImpl () = default;

        GeometryPassSamplerDescriptorSetLayoutImpl ( GeometryPassSamplerDescriptorSetLayoutImpl const & ) = delete;

        GeometryPassSamplerDescriptorSetLayoutImpl& operator = (
            GeometryPassSamplerDescriptorSetLayoutImpl const &
        ) = delete;

        GeometryPassSamplerDescriptorSetLayoutImpl ( GeometryPassSamplerDescriptorSetLayoutImpl && ) = delete;

        GeometryPassSamplerDescriptorSetLayoutImpl& operator = (
            GeometryPassSamplerDescriptorSetLayoutImpl &&
        ) = delete;

        ~GeometryPassSamplerDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

void GeometryPassSamplerDescriptorSetLayoutImpl::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::GeometryPassSamplerDescriptorSetLayoutImpl::_layout" )
}

bool GeometryPassSamplerDescriptorSetLayoutImpl::Init ( VkDevice device ) noexcept
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
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_layout ),
        "pbr::GeometryPassSamplerDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::GeometryPassSamplerDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static GeometryPassSamplerDescriptorSetLayoutImpl g_samplerDescriptorSetLayout {};

//----------------------------------------------------------------------------------------------------------------------

void GeometryPassSamplerDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_samplerDescriptorSetLayout.Destroy ( device );
}

bool GeometryPassSamplerDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_samplerDescriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout GeometryPassSamplerDescriptorSetLayout::GetLayout () const noexcept
{
    return g_samplerDescriptorSetLayout._layout;
}

} // namespace pbr
