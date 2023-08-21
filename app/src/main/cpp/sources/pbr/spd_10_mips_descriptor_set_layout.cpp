#include <pbr/spd_10_mips_descriptor_set_layout.hpp>
#include <pbr/spd_descriptor_set_layout.hpp>


namespace pbr {

namespace {

SPDDescriptorSetLayout g_descriptorSetLayout ( 7U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void SPD10MipsDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_descriptorSetLayout.Destroy ( device );
}

bool SPD10MipsDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout SPD10MipsDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._layout;
}

} // namespace pbr
