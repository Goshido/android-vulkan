#include <pbr/spd_10_mips_program.hpp>
#include <pbr/spd_10_mips_descriptor_set_layout.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/spd_10_mips.cs.spv";

} // end of anonymous namespace

SPD10MipsProgram::SPD10MipsProgram () noexcept:
    SPDProgram ( "pbr::SPD10MipsProgram", std::make_unique<SPD10MipsDescriptorSetLayout> (), SHADER )
{
    // NOTHING
}

} // namespace pbr
