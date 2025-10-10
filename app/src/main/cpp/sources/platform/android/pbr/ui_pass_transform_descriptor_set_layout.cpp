#include <precompiled_headers.hpp>
#include <platform/android/pbr/ui_pass_transform_descriptor_set_layout.hpp>
#include <platform/android/pbr/ui_program.inc>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

class DescriptorSetLayout final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

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
    if ( _references > 0U && --_references == 0U )
    {
        vkDestroyDescriptorSetLayout ( device, std::exchange ( _layout, VK_NULL_HANDLE ), nullptr );
    }
}

bool DescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    if ( ++_references != 1U )
        return true;

    constexpr static VkDescriptorSetLayoutBinding binding
    {
        .binding = BIND_TRANSFORM,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1U,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
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
        vkCreateDescriptorSetLayout ( device, &info, nullptr, &_layout ),
        "pbr::UIPassTransformDescriptorSetLayout::Init",
        "Can't create descriptor set layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "UI pass transform" )
    return true;
}

DescriptorSetLayout g_descriptorSetLayout {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void UIPassTransformDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    if ( _init )
    {
        g_descriptorSetLayout.Destroy ( device );
        _init = false;
    }
}

bool UIPassTransformDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    _init = true;
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout &UIPassTransformDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._layout;
}

} // namespace pbr
