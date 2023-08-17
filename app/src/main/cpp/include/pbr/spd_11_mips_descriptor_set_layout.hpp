#ifndef PBR_SPD_11_MIPS_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_SPD_11_MIPS_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class SPD11MipsDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        SPD11MipsDescriptorSetLayout () = default;

        SPD11MipsDescriptorSetLayout ( SPD11MipsDescriptorSetLayout const & ) = delete;
        SPD11MipsDescriptorSetLayout &operator = ( SPD11MipsDescriptorSetLayout const & ) = delete;

        SPD11MipsDescriptorSetLayout ( SPD11MipsDescriptorSetLayout && ) = delete;
        SPD11MipsDescriptorSetLayout &operator = ( SPD11MipsDescriptorSetLayout && ) = delete;

        ~SPD11MipsDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_SPD_11_MIPS_DESCRIPTOR_SET_LAYOUT_HPP
