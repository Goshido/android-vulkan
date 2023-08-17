#include <pbr/spd_11_mips_program.hpp>
#include <pbr/spd_11_mips_descriptor_set_layout.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/spd_11_mips.cs.spv";

} // end of anonymous namespace

SPD11MipsProgram::SPD11MipsProgram () noexcept:
    SPDProgram ( "pbr::SPD11MipsProgram", std::make_unique<SPD11MipsDescriptorSetLayout> (), SHADER )
{
    // NOTHING
}

} // namespace pbr
