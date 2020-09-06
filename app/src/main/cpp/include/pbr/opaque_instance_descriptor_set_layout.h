#ifndef PBR_OPAQUE_INSTANCE_DESCRIPTOR_SET_LAYOUT_H
#define PBR_OPAQUE_INSTANCE_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class OpaqueInstanceDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        OpaqueInstanceDescriptorSetLayout () = default;

        OpaqueInstanceDescriptorSetLayout ( OpaqueInstanceDescriptorSetLayout const &other ) = delete;
        OpaqueInstanceDescriptorSetLayout& operator = ( OpaqueInstanceDescriptorSetLayout const &other ) = delete;

        OpaqueInstanceDescriptorSetLayout ( OpaqueInstanceDescriptorSetLayout &&other ) = delete;
        OpaqueInstanceDescriptorSetLayout& operator = ( OpaqueInstanceDescriptorSetLayout &&other ) = delete;

        ~OpaqueInstanceDescriptorSetLayout () override = default;

        void Destroy ( android_vulkan::Renderer &renderer ) override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const override;
};

} // namespace pbr


#endif // PBR_OPAQUE_INSTANCE_DESCRIPTOR_SET_LAYOUT_H
