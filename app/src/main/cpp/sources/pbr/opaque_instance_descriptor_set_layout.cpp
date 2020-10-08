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
        std::atomic<size_t>         _references;

    public:
        OpaqueInstanceDescriptorSetLayoutImpl () noexcept;

        OpaqueInstanceDescriptorSetLayoutImpl ( OpaqueInstanceDescriptorSetLayoutImpl const &other ) = delete;
        OpaqueInstanceDescriptorSetLayoutImpl& operator = ( OpaqueInstanceDescriptorSetLayoutImpl const &other ) = delete;

        OpaqueInstanceDescriptorSetLayoutImpl ( OpaqueInstanceDescriptorSetLayoutImpl &&other ) = delete;
        OpaqueInstanceDescriptorSetLayoutImpl& operator = ( OpaqueInstanceDescriptorSetLayoutImpl &&other ) = delete;

        ~OpaqueInstanceDescriptorSetLayoutImpl () = default;

        void Destroy ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

OpaqueInstanceDescriptorSetLayoutImpl::OpaqueInstanceDescriptorSetLayoutImpl () noexcept:
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void OpaqueInstanceDescriptorSetLayoutImpl::Destroy ( android_vulkan::Renderer &renderer )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( renderer.GetDevice (), _descriptorSetLayout, nullptr );
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

    VkDescriptorSetLayoutBinding uniformBuffer;
    uniformBuffer.stageFlags = AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT )/* | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT )*/;
    uniformBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBuffer.descriptorCount = 1U;
    uniformBuffer.binding = 0U;
    uniformBuffer.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0U;
    descriptorSetLayoutInfo.bindingCount = 1U;
    descriptorSetLayoutInfo.pBindings = &uniformBuffer;

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
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

void OpaqueInstanceDescriptorSetLayout::Destroy ( android_vulkan::Renderer &renderer )
{
    g_opaqueInstanceDescriptorSetLayout.Destroy ( renderer );
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
