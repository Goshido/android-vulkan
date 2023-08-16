#ifndef PBR_SPD_12_MIPS_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_SPD_12_MIPS_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class SPD12MipsDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        SPD12MipsDescriptorSetLayout () = default;

        SPD12MipsDescriptorSetLayout ( SPD12MipsDescriptorSetLayout const & ) = delete;
        SPD12MipsDescriptorSetLayout &operator = ( SPD12MipsDescriptorSetLayout const & ) = delete;

        SPD12MipsDescriptorSetLayout ( SPD12MipsDescriptorSetLayout && ) = delete;
        SPD12MipsDescriptorSetLayout &operator = ( SPD12MipsDescriptorSetLayout && ) = delete;

        ~SPD12MipsDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_SPD_12_MIPS_DESCRIPTOR_SET_LAYOUT_HPP
