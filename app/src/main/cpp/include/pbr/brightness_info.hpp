#ifndef PBR_BRIGHTNESS_INFO_HPP
#define PBR_BRIGHTNESS_INFO_HPP


namespace pbr {

class BrightnessInfo final
{
    public:
        [[maybe_unused]] float      _brightnessFactor = 1.0F;
        [[maybe_unused]] bool       _isDefaultBrightness = true;

    public:
        explicit BrightnessInfo () = default;

        BrightnessInfo ( BrightnessInfo const & ) = delete;
        BrightnessInfo &operator = ( BrightnessInfo const & ) = delete;

        BrightnessInfo ( BrightnessInfo && ) = delete;
        BrightnessInfo &operator = ( BrightnessInfo && ) = delete;

        // Brightness balance should be in range [-1.0F, 1.0F].
        explicit BrightnessInfo ( float brightnessBalance ) noexcept;

        ~BrightnessInfo () = default;
};

} // namespace pbr


#endif // PBR_BRIGHTNESS_INFO_HPP
