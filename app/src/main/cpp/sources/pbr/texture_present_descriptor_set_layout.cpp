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

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

TexturePresentDescriptorSetLayoutImpl::TexturePresentDescriptorSetLayoutImpl ():
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void TexturePresentDescriptorSetLayoutImpl::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::TexturePresentDescriptorSetLayoutImpl::_descriptorSetLayout" )
}


bool TexturePresentDescriptorSetLayoutImpl::Init ( VkDevice device ) noexcept
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
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout ),
        "pbr::TexturePresentDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::TexturePresentDescriptorSetLayoutImpl::_descriptorSetLayout" )

    ++_references;
    return true;
}

static TexturePresentDescriptorSetLayoutImpl g_texturePresentDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void TexturePresentDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_texturePresentDescriptorSetLayout.Destroy ( device );
}

bool TexturePresentDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_texturePresentDescriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout TexturePresentDescriptorSetLayout::GetLayout () const noexcept
{
    return g_texturePresentDescriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
