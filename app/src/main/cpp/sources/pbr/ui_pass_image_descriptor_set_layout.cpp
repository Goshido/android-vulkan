#include <pbr/ui_pass_image_descriptor_set_layout.h>
#include <pbr/ui_program.inc>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

class DescriptorSetLayout final
{
    public:
        VkDescriptorSetLayout       _descriptorSetLayout = VK_NULL_HANDLE;

    private:
        std::atomic_size_t          _references = 0U;

    public:
        DescriptorSetLayout () = default;

        DescriptorSetLayout ( DescriptorSetLayout const & ) = delete;
        DescriptorSetLayout &operator = ( DescriptorSetLayout const & ) = delete;

        DescriptorSetLayout ( DescriptorSetLayout && ) = delete;
        DescriptorSetLayout &operator = ( DescriptorSetLayout && ) = delete;

        ~DescriptorSetLayout () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

void DescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::UIPassImageDescriptorSetLayout::_descriptorSetLayout" )
}

bool DescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    constexpr static VkDescriptorSetLayoutBinding binding
    {
        .binding = BIND_IMAGE_TEXTURE,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 1U,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    constexpr VkDescriptorSetLayoutCreateInfo info
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = 1U,
        .pBindings = &binding
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &info, nullptr, &_descriptorSetLayout ),
        "pbr::UIPassImageDescriptorSetLayout::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::UIPassImageDescriptorSetLayout::_descriptorSetLayout" )

    ++_references;
    return true;
}

DescriptorSetLayout g_descriptorSetLayout {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void UIPassImageDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_descriptorSetLayout.Destroy ( device );
}

bool UIPassImageDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout UIPassImageDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
