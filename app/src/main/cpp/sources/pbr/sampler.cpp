#include <pbr/sampler.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

Sampler::Sampler () noexcept:
    _sampler ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool Sampler::Init ( const VkSamplerCreateInfo &info, android_vulkan::Renderer &renderer )
{
    assert ( _sampler == VK_NULL_HANDLE );

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( renderer.GetDevice (), &info, nullptr, &_sampler ),
        "Sampler::Init",
        "Can't create sampler"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Sampler::_sampler" )
    return true;
}

void Sampler::Destroy ( android_vulkan::Renderer &renderer )
{
    if ( _sampler == VK_NULL_HANDLE )
        return;

    vkDestroySampler ( renderer.GetDevice (), _sampler, nullptr );
    _sampler = VK_NULL_HANDLE;
    AV_UNREGISTER_SAMPLER ( "Sampler::_sampler" )
}

VkSampler Sampler::GetSampler () const
{
    return _sampler;
}

} // namespace pbr
