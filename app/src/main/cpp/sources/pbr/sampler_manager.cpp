#include <pbr/sampler_manager.h>


namespace pbr {

bool SamplerManager::Init ( VkDevice device ) noexcept
{
    _pointSampler = std::make_shared<Sampler> ();

    constexpr VkSamplerCreateInfo pointSamplerInfo
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

    if ( !_pointSampler->Init ( device, pointSamplerInfo ) )
    {
        _pointSampler = nullptr;
        return false;
    }

    _materialSampler = std::make_shared<Sampler> ();

    constexpr VkSamplerCreateInfo materialSamplerInfo
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
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( _materialSampler->Init ( device, materialSamplerInfo ) )
        return true;

    _materialSampler = nullptr;
    return false;
}

void SamplerManager::Destroy ( VkDevice device ) noexcept
{
    if ( _materialSampler )
    {
        _materialSampler->Destroy ( device );
        _materialSampler = nullptr;
    }

    if ( !_pointSampler )
        return;

    _pointSampler->Destroy ( device );
    _pointSampler = nullptr;
}

SamplerRef const &SamplerManager::GetMaterialSampler () const noexcept
{
    return _materialSampler;
}

SamplerRef const &SamplerManager::GetPointSampler () const noexcept
{
    return _pointSampler;
}

} // namespace pbr
