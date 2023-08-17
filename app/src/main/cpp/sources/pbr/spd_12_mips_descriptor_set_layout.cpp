#include <pbr/spd_12_mips_descriptor_set_layout.hpp>
#include <pbr/spd_descriptor_set_layout.hpp>


namespace pbr {

namespace {

SPDDescriptorSetLayout g_descriptorSetLayout ( 10U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void SPD12MipsDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_descriptorSetLayout.Destroy ( device );
}

bool SPD12MipsDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout SPD12MipsDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._layout;
}

} // namespace pbr
