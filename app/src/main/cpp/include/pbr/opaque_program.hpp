#ifndef PBR_OPAQUE_PROGRAM_HPP
#define PBR_OPAQUE_PROGRAM_HPP


#include "geometry_pass_program.hpp"


namespace pbr {

class OpaqueProgram final : public GeometryPassProgram
{
    public:
        OpaqueProgram () noexcept;

        OpaqueProgram ( OpaqueProgram const & ) = delete;
        OpaqueProgram &operator = ( OpaqueProgram const & ) = delete;

        OpaqueProgram ( OpaqueProgram && ) = delete;
        OpaqueProgram &operator = ( OpaqueProgram && ) = delete;

        ~OpaqueProgram () override = default;
};

} // namespace pbr


#endif // PBR_OPAQUE_PROGRAM_HPP
