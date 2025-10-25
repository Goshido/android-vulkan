#include <precompiled_headers.hpp>
#include <pbr/exposure_specialization.hpp>


namespace pbr {

ExposureSpecialization::ExposureSpecialization ( VkExtent3D &dispatch,
    VkExtent2D &mipChainResolution,
    VkExtent2D const &imageResolution
) noexcept
{
    constexpr uint32_t multipleOf64 = 6U;

    dispatch =
    {
        .width = imageResolution.width >> multipleOf64,
        .height = imageResolution.height >> multipleOf64,
        .depth = 1U
    };

    _lastWorkgroupIndex = dispatch.width * dispatch.height - 1U;

    // mip 0
    VkExtent2D r
    {
        .width = imageResolution.width >> 1U,
        .height = imageResolution.height >> 1U
    };

    mipChainResolution = r;

    constexpr auto reduce = [] ( uint32_t v ) noexcept -> uint32_t {
        return std::max ( 1U, v >> 1U );
    };

    auto const nextMip = [ & ] ( VkExtent2D resolution ) noexcept -> VkExtent2D {
        return VkExtent2D
        {
            .width = reduce ( resolution.width ),
            .height = reduce ( resolution.height )
        };
    };

    // Making mip 5...
    for ( uint8_t i = 0U; i < 5U; ++i )
        r = nextMip ( r );

    _mip5Resolution = r;
    _normalizeW = 1.0F / static_cast<float> ( r.width );
    _normalizeH = 1.0F / static_cast<float> ( r.height );
}

} // namespace pbr
