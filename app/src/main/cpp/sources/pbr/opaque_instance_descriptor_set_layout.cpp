#include <pbr/opaque_instance_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class OpaqueInstanceDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _descriptorSetLayout;

    private:
        std::atomic_size_t          _references;

    public:
        OpaqueInstanceDescriptorSetLayoutImpl () noexcept;

        OpaqueInstanceDescriptorSetLayoutImpl ( OpaqueInstanceDescriptorSetLayoutImpl const & ) = delete;
        OpaqueInstanceDescriptorSetLayoutImpl& operator = ( OpaqueInstanceDescriptorSetLayoutImpl const & ) = delete;

        OpaqueInstanceDescriptorSetLayoutImpl ( OpaqueInstanceDescriptorSetLayoutImpl && ) = delete;
        OpaqueInstanceDescriptorSetLayoutImpl& operator = ( OpaqueInstanceDescriptorSetLayoutImpl && ) = delete;

        ~OpaqueInstanceDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

OpaqueInstanceDescriptorSetLayoutImpl::OpaqueInstanceDescriptorSetLayoutImpl () noexcept:
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void OpaqueInstanceDescriptorSetLayoutImpl::Destroy ( VkDevice device )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueInstanceDescriptorSetLayoutImpl::_descriptorSetLayout" )
}

bool OpaqueInstanceDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
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
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
        "OpaqueInstanceDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueInstanceDescriptorSetLayoutImpl::_descriptorSetLayout" )

    ++_references;
    return true;
}

static OpaqueInstanceDescriptorSetLayoutImpl g_opaqueInstanceDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void OpaqueInstanceDescriptorSetLayout::Destroy ( VkDevice device )
{
    g_opaqueInstanceDescriptorSetLayout.Destroy ( device );
}

bool OpaqueInstanceDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_opaqueInstanceDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout OpaqueInstanceDescriptorSetLayout::GetLayout () const
{
    return g_opaqueInstanceDescriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
