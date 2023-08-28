#ifndef PBR_UI_PASS_IMAGE_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_UI_PASS_IMAGE_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class UIPassImageDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        UIPassImageDescriptorSetLayout () = default;

        UIPassImageDescriptorSetLayout ( UIPassImageDescriptorSetLayout const & ) = delete;
        UIPassImageDescriptorSetLayout &operator = ( UIPassImageDescriptorSetLayout const & ) = delete;

        UIPassImageDescriptorSetLayout ( UIPassImageDescriptorSetLayout && ) = delete;
        UIPassImageDescriptorSetLayout &operator = ( UIPassImageDescriptorSetLayout && ) = delete;

        ~UIPassImageDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_UI_PASS_IMAGE_DESCRIPTOR_SET_LAYOUT_HPP
