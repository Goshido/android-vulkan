#ifndef PBR_SPD_11_MIPS_PROGRAM_HPP
#define PBR_SPD_11_MIPS_PROGRAM_HPP


#include "spd_program.hpp"


namespace pbr {

class SPD11MipsProgram final : public SPDProgram
{
    public:
        explicit SPD11MipsProgram () noexcept;

        SPD11MipsProgram ( SPD11MipsProgram const & ) = delete;
        SPD11MipsProgram &operator = ( SPD11MipsProgram const & ) = delete;

        SPD11MipsProgram ( SPD11MipsProgram && ) = delete;
        SPD11MipsProgram &operator = ( SPD11MipsProgram && ) = delete;

        ~SPD11MipsProgram () override = default;
};

} // namespace pbr


#endif // PBR_SPD_11_MIPS_PROGRAM_HPP
