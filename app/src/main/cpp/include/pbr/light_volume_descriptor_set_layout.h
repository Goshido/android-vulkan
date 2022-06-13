#ifndef PBR_LIGHT_VOLUME_DESCRIPTOR_SET_LAYOUT_H
#define PBR_LIGHT_VOLUME_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class LightVolumeDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        LightVolumeDescriptorSetLayout () = default;

        LightVolumeDescriptorSetLayout ( LightVolumeDescriptorSetLayout const & ) = delete;
        LightVolumeDescriptorSetLayout& operator = ( LightVolumeDescriptorSetLayout const & ) = delete;

        LightVolumeDescriptorSetLayout ( LightVolumeDescriptorSetLayout && ) = delete;
        LightVolumeDescriptorSetLayout& operator = ( LightVolumeDescriptorSetLayout && ) = delete;

        ~LightVolumeDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_LIGHT_VOLUME_DESCRIPTOR_SET_LAYOUT_H
