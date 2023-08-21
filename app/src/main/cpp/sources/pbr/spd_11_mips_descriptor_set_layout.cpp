#include <pbr/spd_11_mips_descriptor_set_layout.hpp>
#include <pbr/spd_descriptor_set_layout.hpp>


namespace pbr {

namespace {

SPDDescriptorSetLayout g_descriptorSetLayout ( 8U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void SPD11MipsDescriptorSetLayout::Destroy ( VkDevice device ) noexcept
{
    g_descriptorSetLayout.Destroy ( device );
}

bool SPD11MipsDescriptorSetLayout::Init ( VkDevice device ) noexcept
{
    return g_descriptorSetLayout.Init ( device );
}

VkDescriptorSetLayout SPD11MipsDescriptorSetLayout::GetLayout () const noexcept
{
    return g_descriptorSetLayout._layout;
}

} // namespace pbr
