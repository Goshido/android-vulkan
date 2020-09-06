#include <pbr/opaque_texture_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class OpaqueTextureDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _descriptorSetLayout;

    private:
        std::atomic<size_t>         _references;

    public:
        OpaqueTextureDescriptorSetLayoutImpl ();

        OpaqueTextureDescriptorSetLayoutImpl ( OpaqueTextureDescriptorSetLayoutImpl const &other ) = delete;
        OpaqueTextureDescriptorSetLayoutImpl& operator = ( OpaqueTextureDescriptorSetLayoutImpl const &other ) = delete;

        OpaqueTextureDescriptorSetLayoutImpl ( OpaqueTextureDescriptorSetLayoutImpl &&other ) = delete;
        OpaqueTextureDescriptorSetLayoutImpl& operator = ( OpaqueTextureDescriptorSetLayoutImpl &&other ) = delete;

        ~OpaqueTextureDescriptorSetLayoutImpl () = default;

        void Destroy ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

OpaqueTextureDescriptorSetLayoutImpl::OpaqueTextureDescriptorSetLayoutImpl ():
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void OpaqueTextureDescriptorSetLayoutImpl::Destroy ( android_vulkan::Renderer &renderer )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( renderer.GetDevice (), _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueTextureDescriptorSetLayoutImpl::_descriptorSetLayout" )
}

bool OpaqueTextureDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    VkDescriptorSetLayoutBinding bindingInfo[ 8U ];
    VkDescriptorSetLayoutBinding& albedoTexture = bindingInfo[ 0U ];
    albedoTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoTexture.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    albedoTexture.descriptorCount = 1U;
    albedoTexture.binding = 0U;
    albedoTexture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& albedoSampler = bindingInfo[ 1U ];
    albedoSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoSampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    albedoSampler.descriptorCount = 1U;
    albedoSampler.binding = 1U;
    albedoSampler.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& emissionTexture = bindingInfo[ 2U ];
    emissionTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    emissionTexture.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    emissionTexture.descriptorCount = 1U;
    emissionTexture.binding = 2U;
    emissionTexture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& emissionSampler = bindingInfo[ 3U ];
    emissionSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    emissionSampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    emissionSampler.descriptorCount = 1U;
    emissionSampler.binding = 3U;
    emissionSampler.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalTexture = bindingInfo[ 4U ];
    normalTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalTexture.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    normalTexture.descriptorCount = 1U;
    normalTexture.binding = 4U;
    normalTexture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalSampler = bindingInfo[ 5U ];
    normalSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalSampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    normalSampler.descriptorCount = 1U;
    normalSampler.binding = 5U;
    normalSampler.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& paramTexture = bindingInfo[ 6U ];
    paramTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    paramTexture.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    paramTexture.descriptorCount = 1U;
    paramTexture.binding = 6U;
    paramTexture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& paramSampler = bindingInfo[ 7U ];
    paramSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    paramSampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    paramSampler.descriptorCount = 1U;
    paramSampler.binding = 7U;
    paramSampler.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0U;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t> ( std::size ( bindingInfo ) );
    descriptorSetLayoutInfo.pBindings = bindingInfo;

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
        "OpaqueTextureDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "OpaqueTextureDescriptorSetLayoutImpl::_descriptorSetLayout" )

    ++_references;
    return false;
}

static OpaqueTextureDescriptorSetLayoutImpl g_opaqueTextureDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void OpaqueTextureDescriptorSetLayout::Destroy ( android_vulkan::Renderer &renderer )
{
    g_opaqueTextureDescriptorSetLayout.Destroy ( renderer );
}

bool OpaqueTextureDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_opaqueTextureDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout OpaqueTextureDescriptorSetLayout::GetLayout () const
{
    return g_opaqueTextureDescriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
