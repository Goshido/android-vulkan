#include <pbr/sampler_manager.h>


namespace pbr {

void SamplerManager::FreeResources ( android_vulkan::Renderer &renderer )
{
    if ( _pointSampler )
    {
        _pointSampler->Destroy ( renderer );
        _pointSampler = nullptr;
    }

    for ( auto &sampler : _storage )
    {
        if ( !sampler )
            continue;

        sampler->Destroy ( renderer );
        sampler = nullptr;
    }
}

SamplerRef SamplerManager::GetPointSampler ( android_vulkan::Renderer &renderer )
{
    if ( _pointSampler )
        return _pointSampler;

    _pointSampler = std::make_shared<Sampler> ();

    constexpr VkSamplerCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( !_pointSampler->Init ( info, renderer ) )
        _pointSampler = nullptr;

    return _pointSampler;
}

SamplerRef SamplerManager::GetSampler ( uint8_t mips, android_vulkan::Renderer &renderer )
{
    SamplerRef& target = _storage[ static_cast<size_t> ( mips ) ];

    if ( target )
        return target;

    target = std::make_shared<Sampler> ();

    VkSamplerCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = static_cast<float> ( mips - 1U ),
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( !target->Init ( info, renderer ) )
        target = nullptr;

    return target;
}

} // namespace pbr
