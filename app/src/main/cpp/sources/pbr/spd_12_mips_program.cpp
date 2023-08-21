#include <pbr/spd_12_mips_program.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/spd_12_mips.cs.spv";

} // end of anonymous namespace

SPD12MipsProgram::SPD12MipsProgram () noexcept:
    SPDProgram ( "pbr::SPD12MipsProgram", SHADER )
{
    // NOTHING
}

} // namespace pbr
