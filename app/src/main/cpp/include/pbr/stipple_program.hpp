#ifndef PBR_STIPPLE_PROGRAM_HPP
#define PBR_STIPPLE_PROGRAM_HPP


#include "geometry_pass_program.hpp"


namespace pbr {

class StippleProgram final : public GeometryPassProgram
{
    public:
        StippleProgram () noexcept;

        StippleProgram ( StippleProgram const & ) = delete;
        StippleProgram &operator = ( StippleProgram const & ) = delete;

        StippleProgram ( StippleProgram && ) = delete;
        StippleProgram &operator = ( StippleProgram && ) = delete;

        ~StippleProgram () override = default;
};

} // namespace pbr


#endif // PBR_STIPPLE_PROGRAM_HPP
