#ifndef PBR_POINT_LIGHT_DESCRIPTOR_SET_LAYOUT_H
#define PBR_POINT_LIGHT_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class PointLightDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        PointLightDescriptorSetLayout () noexcept = default;

        PointLightDescriptorSetLayout ( PointLightDescriptorSetLayout const & ) = delete;
        PointLightDescriptorSetLayout& operator = ( PointLightDescriptorSetLayout const & ) = delete;

        PointLightDescriptorSetLayout ( PointLightDescriptorSetLayout && ) = delete;
        PointLightDescriptorSetLayout& operator = ( PointLightDescriptorSetLayout && ) = delete;

        ~PointLightDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_DESCRIPTOR_SET_LAYOUT_H
