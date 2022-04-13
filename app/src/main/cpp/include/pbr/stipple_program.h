#ifndef PBR_STIPPLE_PROGRAM_H
#define PBR_STIPPLE_PROGRAM_H


#include "geometry_pass_program.h"


namespace pbr {

class StippleProgram final : public GeometryPassProgram
{
    public:
        StippleProgram () noexcept;

        StippleProgram ( StippleProgram const & ) = delete;
        StippleProgram& operator = ( StippleProgram const & ) = delete;

        StippleProgram ( StippleProgram && ) = delete;
        StippleProgram& operator = ( StippleProgram && ) = delete;

        ~StippleProgram () override = default;
};

} // namespace pbr


#endif // PBR_STIPPLE_PROGRAM_H
