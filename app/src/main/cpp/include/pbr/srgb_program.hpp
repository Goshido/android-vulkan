#ifndef PBR_SRGB_PROGRAM_HPP
#define PBR_SRGB_PROGRAM_HPP


#include "graphics_program.hpp"


namespace pbr {

class SRGBProgram : public GraphicsProgram
{
    public:
        struct GammaInfo final
        {
            [[maybe_unused]] float      _inverseGamma = 1.0F;
        };

    public:
        SRGBProgram () = delete;

        SRGBProgram ( SRGBProgram const & ) = delete;
        SRGBProgram &operator = ( SRGBProgram const & ) = delete;

        SRGBProgram ( SRGBProgram && ) = delete;
        SRGBProgram &operator = ( SRGBProgram && ) = delete;

        // Brightness balance should be in range [-1.0F, 1.0F].
        [[nodiscard]] static GammaInfo GetGammaInfo ( float brightnessBalance ) noexcept;

    protected:
        explicit SRGBProgram ( std::string_view name ) noexcept;
        ~SRGBProgram () override = default;
};

} // namespace pbr


#endif // PBR_SRGB_PROGRAM_HPP
