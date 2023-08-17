#ifndef PBR_SPD_12_MIPS_PROGRAM_HPP
#define PBR_SPD_12_MIPS_PROGRAM_HPP


#include "spd_program.hpp"


namespace pbr {

class SPD12MipsProgram final : public SPDProgram
{
    public:
        explicit SPD12MipsProgram () noexcept;

        SPD12MipsProgram ( SPD12MipsProgram const & ) = delete;
        SPD12MipsProgram &operator = ( SPD12MipsProgram const & ) = delete;

        SPD12MipsProgram ( SPD12MipsProgram && ) = delete;
        SPD12MipsProgram &operator = ( SPD12MipsProgram && ) = delete;

        ~SPD12MipsProgram () override = default;
};

} // namespace pbr


#endif // PBR_SPD_12_MIPS_PROGRAM_HPP
