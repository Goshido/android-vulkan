#ifndef PBR_SPD_10_MIPS_PROGRAM_HPP
#define PBR_SPD_10_MIPS_PROGRAM_HPP


#include "spd_program.hpp"


namespace pbr {

class SPD10MipsProgram final : public SPDProgram
{
    public:
        explicit SPD10MipsProgram () noexcept;

        SPD10MipsProgram ( SPD10MipsProgram const & ) = delete;
        SPD10MipsProgram &operator = ( SPD10MipsProgram const & ) = delete;

        SPD10MipsProgram ( SPD10MipsProgram && ) = delete;
        SPD10MipsProgram &operator = ( SPD10MipsProgram && ) = delete;

        ~SPD10MipsProgram () override = default;
};

} // namespace pbr


#endif // PBR_SPD_10_MIPS_PROGRAM_HPP
