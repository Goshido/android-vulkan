#include <pbr/sampler.hpp>
#include <av_assert.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

bool Sampler::Init ( VkDevice device, VkSamplerCreateInfo const &info ) noexcept
{
    AV_ASSERT ( _sampler == VK_NULL_HANDLE )

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( device, &info, nullptr, &_sampler ),
        "pbr::Sampler::Init",
        "Can't create sampler"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_REGISTER_SAMPLER ( "pbr::Sampler::_sampler" )
    return true;
}

void Sampler::Destroy ( VkDevice device ) noexcept
{
    if ( _sampler == VK_NULL_HANDLE )
        return;

    vkDestroySampler ( device, _sampler, nullptr );
    _sampler = VK_NULL_HANDLE;
    AV_UNREGISTER_SAMPLER ( "pbr::Sampler::_sampler" )
}

VkSampler Sampler::GetSampler () const noexcept
{
    return _sampler;
}

} // namespace pbr
