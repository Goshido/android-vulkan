#ifndef PBR_SPD_10_MIPS_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_SPD_10_MIPS_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class SPD10MipsDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        SPD10MipsDescriptorSetLayout () = default;

        SPD10MipsDescriptorSetLayout ( SPD10MipsDescriptorSetLayout const & ) = delete;
        SPD10MipsDescriptorSetLayout &operator = ( SPD10MipsDescriptorSetLayout const & ) = delete;

        SPD10MipsDescriptorSetLayout ( SPD10MipsDescriptorSetLayout && ) = delete;
        SPD10MipsDescriptorSetLayout &operator = ( SPD10MipsDescriptorSetLayout && ) = delete;

        ~SPD10MipsDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_SPD_10_MIPS_DESCRIPTOR_SET_LAYOUT_HPP
