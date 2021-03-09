#include <pbr/lightup_common_descriptor_set_layout.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

class LightupCommonDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout;

    private:
        std::atomic_size_t          _references;

    public:
        LightupCommonDescriptorSetLayoutImpl () noexcept;

        LightupCommonDescriptorSetLayoutImpl ( LightupCommonDescriptorSetLayoutImpl const & ) = delete;
        LightupCommonDescriptorSetLayoutImpl& operator = ( LightupCommonDescriptorSetLayoutImpl const & ) = delete;

        LightupCommonDescriptorSetLayoutImpl ( LightupCommonDescriptorSetLayoutImpl && ) = delete;
        LightupCommonDescriptorSetLayoutImpl& operator = ( LightupCommonDescriptorSetLayoutImpl && ) = delete;

        ~LightupCommonDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device );
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
};

LightupCommonDescriptorSetLayoutImpl::LightupCommonDescriptorSetLayoutImpl () noexcept:
    _layout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void LightupCommonDescriptorSetLayoutImpl::Destroy ( VkDevice device )
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "LightupCommonDescriptorSetLayoutImpl::_layout" )
}

bool LightupCommonDescriptorSetLayoutImpl::Init ( android_vulkan::Renderer &renderer )
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
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 2U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 3U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 4U,
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
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetLayoutInfo, nullptr, &_layout ),
        "LightupCommonDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "LightupCommonDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static LightupCommonDescriptorSetLayoutImpl g_lightupCommonDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void LightupCommonDescriptorSetLayout::Destroy ( VkDevice device )
{
    g_lightupCommonDescriptorSetLayout.Destroy ( device );
}

bool LightupCommonDescriptorSetLayout::Init ( android_vulkan::Renderer &renderer )
{
    return g_lightupCommonDescriptorSetLayout.Init ( renderer );
}

VkDescriptorSetLayout LightupCommonDescriptorSetLayout::GetLayout () const
{
    return g_lightupCommonDescriptorSetLayout._layout;
}

} // namespace pbr
