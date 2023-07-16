#include <pbr/point_light_shadowmap_generator.inc>
#include <pbr/point_light_shadowmap_generator_descriptor_set_layout.hpp>
#include <vulkan_utils.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE


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
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::PointLightShadowmapGeneratorDescriptorSetLayout::_layout" )
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
        .binding = BIND_INSTANCE_DATA,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1U,
        .stageFlags = AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ),
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
        "pbr::PointLightShadowmapGeneratorDescriptorSetLayout::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::PointLightShadowmapGeneratorDescriptorSetLayout::_layout" )

    ++_references;
    return true;
}

DescriptorSetLayout g_descriptorSetLayout {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void PointLightShadowmapGeneratorDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_descriptorSetLayout.Destroy ( device );
}

bool PointLightShadowmapGeneratorDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout PointLightShadowmapGeneratorDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._layout;
}

} // namespace pbr