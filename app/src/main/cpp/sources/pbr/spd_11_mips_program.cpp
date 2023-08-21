#include <pbr/spd_11_mips_program.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/spd_11_mips.cs.spv";

} // end of anonymous namespace

SPD11MipsProgram::SPD11MipsProgram () noexcept:
    SPDProgram ( "pbr::SPD11MipsProgram", SHADER )
{
    // NOTHING
}

} // namespace pbr
