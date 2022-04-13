#include <pbr/sampler.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

bool Sampler::Init ( android_vulkan::Renderer &renderer, VkSamplerCreateInfo const &info ) noexcept
{
    assert ( _sampler == VK_NULL_HANDLE );

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( renderer.GetDevice (), &info, nullptr, &_sampler ),
        "pbr::Sampler::Init",
        "Can't create sampler"
    );

    if ( !result )
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
