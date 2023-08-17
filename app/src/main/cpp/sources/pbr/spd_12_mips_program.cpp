#include <pbr/spd_12_mips_program.hpp>
#include <pbr/spd_12_mips_descriptor_set_layout.hpp>


namespace pbr {

namespace {

constexpr char const* SHADER = "shaders/spd_12_mips.cs.spv";

} // end of anonymous namespace

SPD12MipsProgram::SPD12MipsProgram () noexcept:
    SPDProgram ( "pbr::SPD12MipsProgram", std::make_unique<SPD12MipsDescriptorSetLayout> (), SHADER )
{
    // NOTHING
}

} // namespace pbr
