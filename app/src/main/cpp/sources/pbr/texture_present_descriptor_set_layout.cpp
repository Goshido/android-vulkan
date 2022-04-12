#include <pbr/texture_present_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class TexturePresentDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _descriptorSetLayout;

    private:
        std::atomic<size_t>         _references;

    public:
        TexturePresentDescriptorSetLayoutImpl ();

        TexturePresentDescriptorSetLayoutImpl ( TexturePresentDescriptorSetLayoutImpl const &other ) = delete;

        TexturePresentDescriptorSetLayoutImpl& operator = (
            TexturePresentDescriptorSetLayoutImpl const &other
        ) = delete;

        TexturePresentDescriptorSetLayoutImpl ( TexturePresentDescriptorSetLayoutImpl &&other ) = delete;
        TexturePresentDescriptorSetLayoutImpl& operator = ( TexturePresentDescriptorSetLayoutImpl &&other ) = delete;

        ~TexturePresentDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

TexturePresentDescriptorSetLayoutImpl::TexturePresentDescriptorSetLayoutImpl ():
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void TexturePresentDescriptorSetLayoutImpl::Destroy ( VkDevice device )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "TexturePresentDescriptorSetLayoutImpl::_descriptorSetLayout" )
}


bool TexturePresentDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    VkDescriptorSetLayoutBinding bindingInfo[ 2U ];
    VkDescriptorSetLayoutBinding& textureBind = bindingInfo[ 0U ];
    textureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    textureBind.descriptorCount = 1U;
    textureBind.binding = 0U;
    textureBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& samplerBind = bindingInfo[ 1U ];
    samplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBind.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerBind.descriptorCount = 1U;
    samplerBind.binding = 1U;
    samplerBind.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.pNext = nullptr;
    descriptorSetLayoutInfo.flags = 0U;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t> ( std::size ( bindingInfo ) );
    descriptorSetLayoutInfo.pBindings = bindingInfo;

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
        "pbr::TexturePresentDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "TexturePresentDescriptorSetLayoutImpl::_descriptorSetLayout" )

    ++_references;
    return true;
}

static TexturePresentDescriptorSetLayoutImpl g_texturePresentDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void TexturePresentDescriptorSetLayout::Destroy ( VkDevice device )
{
    g_texturePresentDescriptorSetLayout.Destroy ( device );
}

bool TexturePresentDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_texturePresentDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout TexturePresentDescriptorSetLayout::GetLayout () const
{
    return g_texturePresentDescriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
