#ifndef PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class LightupCommonDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        LightupCommonDescriptorSetLayout () = default;

        LightupCommonDescriptorSetLayout ( LightupCommonDescriptorSetLayout const & ) = delete;
        LightupCommonDescriptorSetLayout &operator = ( LightupCommonDescriptorSetLayout const & ) = delete;

        LightupCommonDescriptorSetLayout ( LightupCommonDescriptorSetLayout && ) = delete;
        LightupCommonDescriptorSetLayout &operator = ( LightupCommonDescriptorSetLayout && ) = delete;

        ~LightupCommonDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_LAYOUT_HPP
