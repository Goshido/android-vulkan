#include <precompiled_headers.hpp>
#include <pbr/exposure_specialization.hpp>
#include <texture2D.hpp>


namespace pbr {

ExposureSpecialization::ExposureSpecialization ( VkExtent2D const &imageResolution ) noexcept
{
    constexpr uint32_t multipleOf64 = 6U;

    constexpr auto adjustMip0 = [] ( uint32_t size ) constexpr -> uint32_t {
        constexpr uint32_t block = 1U << multipleOf64;
        constexpr uint32_t roundUP = ( block ) - 1U;

        return std::max ( block,
            std::min ( 4095U, std::max ( 1U, ( size + roundUP ) >> multipleOf64 ) << multipleOf64 )
        );
    };

    _mip0Resolution = 
    {
        .width = adjustMip0 ( imageResolution.width ),
        .height = adjustMip0 ( imageResolution.height )
    };

    _dispatch =
    {
        .width = _mip0Resolution.width >> multipleOf64,
        .height = _mip0Resolution.height >> multipleOf64,
        .depth = 1U
    };

    _lastWorkgroupIndex = _dispatch.width * _dispatch.height - 1U;

    // mip 1
    VkExtent2D r
    {
        .width = _mip0Resolution.width >> 1U,
        .height = _mip0Resolution.height >> 1U
    };

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
