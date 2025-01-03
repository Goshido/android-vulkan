#include <precompiled_headers.hpp>
#include <pbr/sampler.hpp>
#include <av_assert.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

bool Sampler::Init ( VkDevice device, VkSamplerCreateInfo const &info, [[maybe_unused]] char const* name ) noexcept
{
    AV_ASSERT ( _sampler == VK_NULL_HANDLE )

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( device, &info, nullptr, &_sampler ),
        "pbr::Sampler::Init",
        "Can't create sampler"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _sampler, VK_OBJECT_TYPE_SAMPLER, "%s", name )
    return true;
}

void Sampler::Destroy ( VkDevice device ) noexcept
{
    if ( _sampler == VK_NULL_HANDLE )
        return;

    vkDestroySampler ( device, _sampler, nullptr );
    _sampler = VK_NULL_HANDLE;
}

VkSampler Sampler::GetSampler () const noexcept
{
    return _sampler;
}

} // namespace pbr
