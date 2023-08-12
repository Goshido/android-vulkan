#include <pbr/average_brightness_pass.hpp>


namespace pbr {

bool AverageBrightnessPass::Init ( android_vulkan::Renderer &/*renderer*/ ) noexcept
{
    return true;
}

void AverageBrightnessPass::Destroy ( android_vulkan::Renderer &/*renderer*/ ) noexcept
{
    // TODO
}

bool AverageBrightnessPass::SetTarget ( android_vulkan::Renderer &/*renderer*/,
    android_vulkan::Texture2D const &/*hdrImage*/ ) noexcept
{
    // TODO
    return true;
}

VkExtent2D AverageBrightnessPass::AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept
{
    auto const adjust = [] ( uint32_t size ) constexpr -> uint32_t {
        constexpr uint32_t blockSizeShift = 6U;
        constexpr uint32_t roundUP = ( 1U << blockSizeShift ) - 1U;

        return std::min ( 2048U, std::max ( 1U, ( size + roundUP ) >> blockSizeShift ) << blockSizeShift );
    };

    return VkExtent2D
    {
        .width = adjust ( desiredResolution.width ),
        .height = adjust ( desiredResolution.height )
    };
}

} // namespace pbr
