#ifndef PBR_LIGHT_LIGHTUP_BASE_PROGRAM_H
#define PBR_LIGHT_LIGHTUP_BASE_PROGRAM_H


#include <vulkan_utils.h>
#include "program.h"


namespace pbr {

class LightLightupBaseProgram : public Program
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct [[maybe_unused]] ViewData final
        {
            [[maybe_unused]] GXMat4                     _toView;
            [[maybe_unused]] GXVec2                     _invResolutionFactor;
            [[maybe_unused]] GXVec2                     _padding0_0;
        };

        AV_DX_ALIGNMENT_END

    public:
        LightLightupBaseProgram () = delete;

        LightLightupBaseProgram ( LightLightupBaseProgram const & ) = delete;
        LightLightupBaseProgram& operator = ( LightLightupBaseProgram const & ) = delete;

        LightLightupBaseProgram ( LightLightupBaseProgram && ) = delete;
        LightLightupBaseProgram& operator = ( LightLightupBaseProgram && ) = delete;

    protected:
        explicit LightLightupBaseProgram ( std::string &&name ) noexcept;
        ~LightLightupBaseProgram () override = default;
};

} // namespace pbr


#endif // PBR_LIGHT_LIGHTUP_BASE_PROGRAM_H
