#ifndef PBR_UI_PASS_COMMON_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_UI_PASS_COMMON_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class UIPassCommonDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        UIPassCommonDescriptorSetLayout () = default;

        UIPassCommonDescriptorSetLayout ( UIPassCommonDescriptorSetLayout const & ) = delete;
        UIPassCommonDescriptorSetLayout &operator = ( UIPassCommonDescriptorSetLayout const & ) = delete;

        UIPassCommonDescriptorSetLayout ( UIPassCommonDescriptorSetLayout && ) = delete;
        UIPassCommonDescriptorSetLayout &operator = ( UIPassCommonDescriptorSetLayout && ) = delete;

        ~UIPassCommonDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_UI_PASS_COMMON_DESCRIPTOR_SET_LAYOUT_HPP
