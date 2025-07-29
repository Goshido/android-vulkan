#ifndef PBR_SRGB_PROGRAM_HPP
#define PBR_SRGB_PROGRAM_HPP


#include <platform/android/pbr/graphics_program.hpp>


namespace pbr {

class BrightnessProgram : public android::GraphicsProgram
{
    public:
        struct BrightnessInfo final
        {
            [[maybe_unused]] float      _brightnessFactor = 1.0F;
            [[maybe_unused]] bool       _isDefaultBrightness = true;
        };

    public:
        BrightnessProgram () = delete;

        BrightnessProgram ( BrightnessProgram const & ) = delete;
        BrightnessProgram &operator = ( BrightnessProgram const & ) = delete;

        BrightnessProgram ( BrightnessProgram && ) = delete;
        BrightnessProgram &operator = ( BrightnessProgram && ) = delete;

        // Brightness balance should be in range [-1.0F, 1.0F].
        [[nodiscard]] static BrightnessInfo GetBrightnessInfo ( float brightnessBalance ) noexcept;

    protected:
        explicit BrightnessProgram ( std::string_view name ) noexcept;
        ~BrightnessProgram () override = default;
};

} // namespace pbr


#endif // PBR_SRGB_PROGRAM_HPP
