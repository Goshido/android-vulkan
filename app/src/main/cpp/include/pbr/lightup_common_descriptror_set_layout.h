#ifndef PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_LAYOUT_H
#define PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class LightupCommonDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        LightupCommonDescriptorSetLayout () noexcept = default;

        LightupCommonDescriptorSetLayout ( LightupCommonDescriptorSetLayout const & ) = delete;
        LightupCommonDescriptorSetLayout& operator = ( LightupCommonDescriptorSetLayout const & ) = delete;

        LightupCommonDescriptorSetLayout ( LightupCommonDescriptorSetLayout && ) = delete;
        LightupCommonDescriptorSetLayout& operator = ( LightupCommonDescriptorSetLayout && ) = delete;

        ~LightupCommonDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const override;
};

} // namespace pbr


#endif // PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_LAYOUT_H
