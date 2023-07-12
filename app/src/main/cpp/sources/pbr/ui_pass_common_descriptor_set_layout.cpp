#include <pbr/ui_pass_common_descriptor_set_layout.h>
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
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::UIPassCommonDescriptorSetLayout::_descriptorSetLayout" )
}

bool DescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    constexpr static VkDescriptorSetLayoutBinding const bindings[] =
    {
        {
            .binding = BIND_ATLAS_SAMPLER,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_ATLAS_TEXTURE,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_IMAGE_SAMPLER,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    constexpr VkDescriptorSetLayoutCreateInfo info
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindings ) ),
        .pBindings = bindings
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &info, nullptr, &_descriptorSetLayout ),
        "pbr::UIPassCommonDescriptorSetLayout::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::UIPassCommonDescriptorSetLayout::_descriptorSetLayout" )

    ++_references;
    return true;
}

DescriptorSetLayout g_descriptorSetLayout {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void UIPassCommonDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_descriptorSetLayout.Destroy ( device );
}

bool UIPassCommonDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout UIPassCommonDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._descriptorSetLayout;
}

} // namespace pbr
