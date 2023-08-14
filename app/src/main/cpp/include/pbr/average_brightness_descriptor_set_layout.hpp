#ifndef PBR_AVERAGE_BRIGHTNESS_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_AVERAGE_BRIGHTNESS_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class AverageBrightnessDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        AverageBrightnessDescriptorSetLayout () = default;

        AverageBrightnessDescriptorSetLayout ( AverageBrightnessDescriptorSetLayout const & ) = delete;
        AverageBrightnessDescriptorSetLayout &operator = ( AverageBrightnessDescriptorSetLayout const & ) = delete;

        AverageBrightnessDescriptorSetLayout ( AverageBrightnessDescriptorSetLayout && ) = delete;
        AverageBrightnessDescriptorSetLayout &operator = ( AverageBrightnessDescriptorSetLayout && ) = delete;

        ~AverageBrightnessDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_AVERAGE_BRIGHTNESS_DESCRIPTOR_SET_LAYOUT_HPP
