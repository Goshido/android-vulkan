#include <pbr/geometry_pass_instance_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class GeometryPassInstanceDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _descriptorSetLayout = VK_NULL_HANDLE;

    private:
        std::atomic_size_t          _references = 0U;

    public:
        GeometryPassInstanceDescriptorSetLayoutImpl () = default;

        GeometryPassInstanceDescriptorSetLayoutImpl ( GeometryPassInstanceDescriptorSetLayoutImpl const & ) = delete;

        GeometryPassInstanceDescriptorSetLayoutImpl& operator = (
            GeometryPassInstanceDescriptorSetLayoutImpl const &
        ) = delete;

        GeometryPassInstanceDescriptorSetLayoutImpl ( GeometryPassInstanceDescriptorSetLayoutImpl && ) = delete;

        GeometryPassInstanceDescriptorSetLayoutImpl& operator = (
            GeometryPassInstanceDescriptorSetLayoutImpl &&
        ) = delete;

        ~GeometryPassInstanceDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
};

void GeometryPassInstanceDescriptorSetLayoutImpl::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::GeometryPassInstanceDescriptorSetLayoutImpl::_descriptorSetLayout" )
}

bool GeometryPassInstanceDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer ) noexcept
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
            .stageFlags = AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT ),
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
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
        "pbr::GeometryPassInstanceDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::GeometryPassInstanceDescriptorSetLayoutImpl::_descriptorSetLayout" )

    ++_references;
    return true;
}

static GeometryPassInstanceDescriptorSetLayoutImpl g_opaqueInstanceDescriptorSetLayout {};

//----------------------------------------------------------------------------------------------------------------------

void GeometryPassInstanceDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_opaqueInstanceDescriptorSetLayout.Destroy ( device );
}

bool GeometryPassInstanceDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    return g_opaqueInstanceDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout GeometryPassInstanceDescriptorSetLayout::GetLayout () const noexcept
{
    return g_opaqueInstanceDescriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
