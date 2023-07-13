#include <pbr/stub_descriptor_set_layout.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.hpp>


namespace pbr {

class StubDescriptorSetLayoutImpl final
{
    public:
        VkDescriptorSetLayout       _layout;

    private:
        std::atomic_size_t          _references;

    public:
        StubDescriptorSetLayoutImpl () noexcept;

        StubDescriptorSetLayoutImpl ( StubDescriptorSetLayoutImpl const & ) = delete;
        StubDescriptorSetLayoutImpl &operator = ( StubDescriptorSetLayoutImpl const & ) = delete;

        StubDescriptorSetLayoutImpl ( StubDescriptorSetLayoutImpl && ) = delete;
        StubDescriptorSetLayoutImpl &operator = ( StubDescriptorSetLayoutImpl && ) = delete;

        ~StubDescriptorSetLayoutImpl () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

StubDescriptorSetLayoutImpl::StubDescriptorSetLayoutImpl () noexcept:
    _layout ( VK_NULL_HANDLE ),
    _references ( 0U )
{
    // NOTHING
}

void StubDescriptorSetLayoutImpl::Destroy ( VkDevice device ) noexcept
{
    if ( !_references )
        return;

    --_references;

    if ( _references )
        return;

    vkDestroyDescriptorSetLayout ( device, _layout, nullptr );
    _layout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::StubDescriptorSetLayoutImpl::_layout" )
}

bool StubDescriptorSetLayoutImpl::Init ( VkDevice device ) noexcept
{
    if ( _references )
    {
        ++_references;
        return true;
    }

    constexpr VkDescriptorSetLayoutCreateInfo const descriptorSetLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = 0,
        .pBindings = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetLayoutInfo, nullptr, &_layout ),
        "pbr::StubDescriptorSetLayoutImpl::Init",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "pbr::StubDescriptorSetLayoutImpl::_layout" )

    ++_references;
    return true;
}

static StubDescriptorSetLayoutImpl g_stubDescriptorSetLayout;

//----------------------------------------------------------------------------------------------------------------------

void StubDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_stubDescriptorSetLayout.Destroy ( device );
}

bool StubDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_stubDescriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout StubDescriptorSetLayout::GetLayout () const noexcept
{
    return g_stubDescriptorSetLayout._layout;
}

} // namespace pbr
