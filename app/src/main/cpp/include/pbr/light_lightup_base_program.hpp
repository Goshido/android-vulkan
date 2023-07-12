#ifndef PBR_LIGHT_LIGHTUP_BASE_PROGRAM_HPP
#define PBR_LIGHT_LIGHTUP_BASE_PROGRAM_HPP


#include <vulkan_utils.hpp>
#include "program.hpp"


namespace pbr {

class LightLightupBaseProgram : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct ViewData final
        {
            [[maybe_unused]] GXMat4     _cvvToView;
            [[maybe_unused]] GXMat4     _viewToWorld;
            [[maybe_unused]] GXVec2     _invResolutionFactor;
            [[maybe_unused]] GXVec2     _padding0_0;
        };

        AV_DX_ALIGNMENT_END

    public:
        LightLightupBaseProgram () = delete;

        LightLightupBaseProgram ( LightLightupBaseProgram const & ) = delete;
        LightLightupBaseProgram &operator = ( LightLightupBaseProgram const & ) = delete;

        LightLightupBaseProgram ( LightLightupBaseProgram && ) = delete;
        LightLightupBaseProgram &operator = ( LightLightupBaseProgram && ) = delete;

    protected:
        explicit LightLightupBaseProgram ( std::string &&name ) noexcept;
        ~LightLightupBaseProgram () override = default;
};

} // namespace pbr


#endif // PBR_LIGHT_LIGHTUP_BASE_PROGRAM_HPP
