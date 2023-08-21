#ifndef PBR_SPD_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_SPD_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class SPDDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        SPDDescriptorSetLayout () = default;

        SPDDescriptorSetLayout ( SPDDescriptorSetLayout const & ) = delete;
        SPDDescriptorSetLayout &operator = ( SPDDescriptorSetLayout const & ) = delete;

        SPDDescriptorSetLayout ( SPDDescriptorSetLayout && ) = delete;
        SPDDescriptorSetLayout &operator = ( SPDDescriptorSetLayout && ) = delete;

        ~SPDDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_SPD_DESCRIPTOR_SET_LAYOUT_HPP
