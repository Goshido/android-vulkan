#include <pbr/sampler.h>
#include <vulkan_utils.h>


namespace pbr {

Sampler::Sampler ():
    _info {},
    _sampler ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool Sampler::Init ( const VkSamplerCreateInfo &info, android_vulkan::Renderer &renderer )
{
    assert ( _sampler == VK_NULL_HANDLE );

    const bool result = renderer.CheckVkResult ( vkCreateSampler ( renderer.GetDevice (), &info, nullptr, &_sampler ),
        "Sampler::Init",
        "Can't create sampler"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Sampler::_sampler" )
    memcpy ( &_info, &info, sizeof ( _info ) );
    return true;
}

void Sampler::Destroy ( android_vulkan::Renderer &renderer )
{
    assert ( _sampler != VK_NULL_HANDLE );

    vkDestroySampler ( renderer.GetDevice (), _sampler, nullptr );
    _sampler = VK_NULL_HANDLE;
    AV_UNREGISTER_SAMPLER ( "Sampler::_sampler" )
}

VkSampler Sampler::GetSampler () const
{
    return _sampler;
}

} // namespace pbr
