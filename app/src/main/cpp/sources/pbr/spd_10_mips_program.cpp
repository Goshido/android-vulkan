#include <pbr/spd_10_mips_program.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/spd_10_mips.cs.spv";

} // end of anonymous namespace

SPD10MipsProgram::SPD10MipsProgram () noexcept:
    SPDProgram ( "pbr::SPD10MipsProgram", SHADER )
{
    // NOTHING
}

} // namespace pbr
